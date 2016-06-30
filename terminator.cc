#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <mutex>
#include <setjmp.h>
#include <stdexcept>
#include <thread>

// Missing set_unexpected / unexpected_handler, violating noexcept, violating exception specification.

#define LOG(msg, ...) fprintf(stderr, msg "\n", ##__VA_ARGS__)

sigjmp_buf jmpbuf;
volatile intptr_t zero = 0;
bool thread_done = false;
std::mutex m;
std::condition_variable cv;

int main() try {
  std::thread t([]() {
      for (auto sig : {SIGTERM, SIGSEGV, SIGINT, SIGILL, SIGABRT, SIGFPE})
        std::signal(sig, [](int sig) {
            switch (sig) {
              case SIGTERM: LOG("\tSIGTERM"); break;
              case SIGSEGV: LOG("\tSIGSEGV"); break;
              case SIGINT:  LOG("\tSIGINT");  break;
              case SIGILL:  LOG("\tSIGILL");  break;
              case SIGABRT: LOG("\tSIGABRT"); break;
              case SIGFPE:  LOG("\tSIGFPE");  break;
              default:      LOG("Unknown signal %i", sig);
            }
            siglongjmp(jmpbuf, sig);
          });
      std::at_quick_exit([] { LOG("\tQuick exiting"); throw std::runtime_error("from quick exit"); });
      std::atexit([] { LOG("\tExiting"); quick_exit(0); });
      std::set_terminate([] { LOG("\tTerminating"); exit(0); });

      int handling = 0;
      handling = sigsetjmp(jmpbuf, 1);
      switch (handling) {
        case SIGTERM: LOG("deref zero");                 *(int*)zero = 0;
        case SIGSEGV: LOG("raising INT");                std::raise(SIGINT);
        case SIGINT:  LOG("executing illegal instr");    asm("ud2");
        case SIGILL:  LOG("dividing by zero");           *(int*)zero = 1337 / zero;
        case SIGABRT: LOG("raising TERM");               std::raise(SIGTERM);
        case SIGFPE:  LOG("OK that's enough signaling"); break;
        default:      LOG("Unknown handling %i", handling);
        case 0:
          LOG("Going to terminate");
          throw std::runtime_error("from thread");
      }

      { std::lock_guard<std::mutex> lock(m); thread_done = true; }
      cv.notify_one();
      for (;;) std::this_thread::sleep_for(std::chrono::seconds(1));
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [] { return thread_done; });
  LOG("main notified that thread is done");

  throw std::runtime_error("from main");
  return 0;
} catch (const std::exception &e) {
  LOG("Caught '%s'", e.what());
  throw;
}
