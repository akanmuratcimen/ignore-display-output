#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <X11/extensions/Xrandr.h>

struct output {
  char *name;
  int name_length;
  bool primary;
  bool ignore;
};

uint output_count = 0;
struct output outputs[16];

bool
is_ignored(
  int argc,
  char *argv[],
  char *output_name
) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], output_name) == 0) {
      return true;
    }
  }

  return false;
}

void
set_connected_outputs(
  int argc,
  char *argv[]
) {
  Display *display = XOpenDisplay(NULL);
  Window root = RootWindow(display, DefaultScreen(display));
  XRRScreenResources *screen_resources = XRRGetScreenResources(display, root);

  for (int i = 0; i < screen_resources->noutput; i++) {
    XRROutputInfo *output_info =
      XRRGetOutputInfo(
        display,
        screen_resources,
        screen_resources->outputs[i]
      );

    if (output_info->connection != RR_Connected) {
      continue;
    }

    struct output output = {
      .name = output_info->name,
      .name_length = output_info->nameLen,
      .ignore = is_ignored(argc, argv, output_info->name),
      .primary = false
    };

    outputs[output_count++] = output;

    XRRFreeOutputInfo(output_info);
  }

  XRRFreeScreenResources(screen_resources);
  XCloseDisplay(display);
}

char cmd[1024] = "xrandr";

void
append_to_xrandr_command(
  struct output output,
  char *args_template
) {
  size_t len = output.name_length + strlen(args_template) - 1;
  char args[len];
  snprintf(args, len, args_template, output.name);
  strcat(cmd, args);
}

void
prepare_xrandr_command(
  void
) {
  for (uint i = 0; i < output_count; i++) {
    if (outputs[i].primary) {
      append_to_xrandr_command(outputs[i], " --output %s --primary --auto");
    } else {
      append_to_xrandr_command(outputs[i], " --output %s --off");
    }
  }
}

void
run_xrandr_command(
  void
) {
#ifdef DEBUG
  printf("%s\n", cmd);
#else
  system(cmd);
#endif
}

int main(
  int argc,
  char *argv[]
) {
  if (system("which xrandr > /dev/null 2>&1") != 0) {
    printf("xrandr couldn't found.\n");
    return EXIT_FAILURE;
  }

  set_connected_outputs(argc, argv);

  bool is_primary_added = false;

  for (uint i = 0; i < output_count; i++) {
    if (outputs[i].ignore) {
      continue;
    }

    if (is_primary_added) {
      break;
    }

    outputs[i].primary = true;
    is_primary_added = true;
  }

  if (!is_primary_added) {
    outputs[0].primary = true;
  }

  prepare_xrandr_command();
  run_xrandr_command();

  return EXIT_SUCCESS;
}

