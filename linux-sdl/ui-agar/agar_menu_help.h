/*
 *
 */

struct HelpFileNamesType {
  char localename[16];
  char helpfile[64];
}; 

const struct HelpFileNamesType HelpFileNames[] = {
  {"C", "xm7-sdl.info"},
  {"ja", "xm7-sdl.ja.info"},
  {NULL, NULL}
};

const char *Help_DefaultInfoPath = "/usr/local/share/info:/usr/share/info:./info";

