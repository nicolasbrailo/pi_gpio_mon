#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_PINS (CHAR_BIT * sizeof(gpio_reg_t))
#define GPIO_PATH "/dev/gpiomem"
#define GPIO_MEM_SZ 4096
#define GPIO_INPUTS 13

typedef unsigned int gpio_reg_t;

// https://www.cs.uaf.edu/2016/fall/cs301/lecture/11_09_raspberry_pi.html

atomic_bool gUsrStop = false;
void sighandler(int) { gUsrStop = true; }

int main(int argc, char **argv) {
  bool update_in_place = false;
  bool log_change = false;
  long monitor_pin = -1;
  for (int i = 0; i < argc; ++i) {
    if ((strcmp(argv[i], "--update_in_place") == 0) ||
        (strcmp(argv[i], "-u") == 0)) {
      update_in_place = true;
      continue;
    } else if ((strcmp(argv[i], "--log_change") == 0) ||
               (strcmp(argv[i], "-l") == 0)) {
      log_change = true;
      continue;
    } else if ((strcmp(argv[i], "-h") == 0) ||
               (strcmp(argv[i], "--help") == 0)) {
      printf("Usage:\n");
      printf("\t%s [--update_in_place|-u] [--log_change|-l] [PIN]\n", argv[0]);
      printf("\n");
      printf("update_in_place: will refresh state in-place\n");
      printf("log_change: will log the time when a pin change was detected\n");
      printf("If PIN is specified, only that pin is monitored\n");
      return 0;
    }

    errno = 0;
    long pin_arg = strtol(argv[i], NULL, 10);
    if (errno == 0 && pin_arg >= 0 && (size_t)pin_arg <= GPIO_PINS) {
      monitor_pin = pin_arg;
    }
  }

  const int gpio_fd = open(GPIO_PATH, O_RDWR);
  if (gpio_fd < 0) {
    perror("Error fopen GPIO");
    return 1;
  }

  gpio_reg_t *gpio_mem =
      mmap(NULL, GPIO_MEM_SZ, PROT_READ + PROT_WRITE, MAP_SHARED, gpio_fd, 0);
  if (!gpio_mem) {
    perror("Error mmap GPIO");
    close(gpio_fd);
    return 1;
  }

  signal(SIGINT, sighandler);

  printf("CNT ");
  for (size_t i = 0; i < GPIO_PINS; ++i) {
    printf("P%02lu ", i);
  }
  printf("\n");

  gpio_reg_t prev_gpio = 0;
  size_t print_cnt = 0;
  while (!gUsrStop) {
#define COL_NOO "\x1B[0m"
#define COL_RED "\x1B[31m"
    printf("%s%03lu", COL_NOO, print_cnt++);
    gpio_reg_t gpio_ins = gpio_mem[GPIO_INPUTS];

    size_t pin_start = 0;
    size_t pin_end = GPIO_PINS;
    if (monitor_pin != -1) {
      pin_start = monitor_pin;
      pin_end = monitor_pin + 1;
      printf(" PIN %ld =", monitor_pin);
    }

    bool change_detected = false;
    for (size_t i = pin_start; i < pin_end; ++i) {
      bool bit = (gpio_ins & (1 << i)) != 0;
      bool prev_bit = (prev_gpio & (1 << i)) != 0;
      if (bit != prev_bit) {
        printf("%s >%u<", COL_RED, bit);
        change_detected = true;
      } else {
        printf("%s  %u ", COL_NOO, bit);
      }
    }
    if (!update_in_place) {
      printf("\n");
    } else {
      if (log_change && change_detected) {
        printf("\n");
      } else {
        printf("\r");
        fflush(stdout);
      }
    }

    prev_gpio = gpio_ins;
    sleep(1);
#undef COL_NOO
#undef COL_RED
  }

  if (munmap(gpio_mem, GPIO_MEM_SZ) != 0) {
    perror("GPIO close munmap fail");
  }

  if (close(gpio_fd) != 0) {
    perror("GPIO close fd fail");
  }

  return 0;
}
