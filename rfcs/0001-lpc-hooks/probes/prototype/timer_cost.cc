#include <chrono>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <cstring>
static volatile int fired;
static void h(int, siginfo_t*, void*) { fired = 1; }
int main() {
  struct sigevent sev; memset(&sev, 0, sizeof sev);
  sev.sigev_signo = SIGVTALRM; sev.sigev_notify = SIGEV_SIGNAL;
  timer_t id; timer_create(CLOCK_MONOTONIC, &sev, &id);
  struct sigaction sa; sa.sa_sigaction = h; sigemptyset(&sa.sa_mask); sa.sa_flags = SA_SIGINFO; sigaction(SIGVTALRM, &sa, nullptr);
  const int N = 2000000;
  struct itimerspec it{}; it.it_value.tv_sec = 100;
  for (int rep = 0; rep < 3; rep++) {
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; i++) timer_settime(id, 0, &it, nullptr);
    auto t1 = std::chrono::steady_clock::now();
    struct itimerspec o;
    for (int i = 0; i < N; i++) timer_gettime(id, &o);
    auto t2 = std::chrono::steady_clock::now();
    long long acc = 0;
    for (int i = 0; i < N; i++) acc += std::chrono::steady_clock::now().time_since_epoch().count() & 1;
    auto t3 = std::chrono::steady_clock::now();
    auto ns = [](auto a, auto b) { return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count(); };
    printf("timer_settime %lld ns  timer_gettime %lld ns  steady_clock::now %lld ns (acc %lld)\n",
           ns(t0, t1) / N, ns(t1, t2) / N, ns(t2, t3) / N, acc & 1);
  }
}
