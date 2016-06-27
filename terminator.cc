#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <setjmp.h>
#include <stdexcept>

#define LOG(msg, ...) fprintf(stderr, msg "\n", ##__VA_ARGS__)

sigjmp_buf jmpbuf;
volatile intptr_t zero = 0;

void signal_handler(int sig) {
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
}

void terminate_handler() { LOG("\tTerminating"); exit(0); }
void exit_handler() { LOG("\tExiting"); quick_exit(0); }
void quick_exit_handler() { LOG("\tQuick exiting"); throw std::runtime_error("from quick exit"); }

int main() try {
  for (auto sig : {SIGTERM, SIGSEGV, SIGINT, SIGILL, SIGABRT, SIGFPE})
    std::signal(sig, signal_handler);
  std::at_quick_exit(quick_exit_handler);
  std::atexit(exit_handler);
  std::set_terminate(terminate_handler);

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
    throw std::runtime_error("from main");
  }

  LOG("At the end of main");
  return 0;
} catch (const std::exception &e) {
  LOG("Caught '%s'", e.what());
  throw;
}
