#include <cstdio>
#include <cstdlib>
#include <exception>
#include <stdexcept>

void terminate_handler() {
  fprintf(stderr, "Terminating, in handler\n");
  exit(0);
}

void exit_handler() {
  fprintf(stderr, "Exiting, in handler\n");
  quick_exit(0);
}

void quick_exit_handler() {
  fprintf(stderr, "Quick exiting, in handler\n");
  throw std::runtime_error("from quick exit");
}

int main() try {
  std::at_quick_exit(quick_exit_handler);
  std::atexit(exit_handler);
  std::set_terminate(terminate_handler);
  fprintf(stderr, "Going to terminate\n");
  throw std::runtime_error("from main");
} catch(const std::exception &e) {
  fprintf(stderr, "Caught %s\n", e.what());
  throw;
}
