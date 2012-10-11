#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "foo-wm.h"
#include "responses.h"


char * jsonMarks(void) {
  char * marksResponse = malloc(1024);
  sprintf(marksResponse, "{\"marks\":[");
  Mark *n = NULL;
  for(n = markTail; n; n = n -> previous) {
    sprintf(marksResponse, 
        "%s%s\"%s\"",
        marksResponse,
        n != markTail ? "," : "",
        n -> name 
        );
  }

  sprintf(marksResponse, "%s]}", marksResponse);
  realloc(marksResponse, bytesUntilNull(marksResponse));
  return marksResponse;
}


char * jsonTree(Node * node, int level) {
  char 
    *buffer = malloc(5120),
    *label = "", 
    *nestString = "";

  /* Recursivly Crawl Children */
  if (!isClient(node)) {
    nestString = malloc(5120);
    Node *n = NULL; int c = 0;

    int last = -1;
    for (n = node -> child; n; n = n -> next, last++) { }
    n = NULL;

    for (n = node -> child; n; n = n -> next, c++) {
      char *crawlResult = malloc(5120);
      crawlResult = jsonTree(n, level + 1);
      sprintf(nestString, "%s%s%s", 
          c == 0 ? "" : nestString, crawlResult, c != last  ? "," : "");
      free(crawlResult);
    }
  }


  /* Determine Label for sprintf */
  if (!isClient(node)) {
    switch (node -> layout) {
      case VERTICAL   : label = ",\"layout\":\"vertical\""; break;
      case HORIZONTAL : label = ",\"layout\":\"horizontal\""; break;
      case GRID       : label = ",\"layout\":\"grid\""; break;
      case MAX        : label = ",\"layout\":\"max\""; break;
      case FLOAT      : label = ",\"layout\":\"float\""; break;
    }
  }

  /* Print to the buffer */
  sprintf(buffer,
      /* Type (Pointer) (?Layout) (?R/V/F) (?ChildFocus) */
      "{\"node\":\"%p\",\"type\":\"%s\"%s%s%s%s%s,\"children\":[%s]}\0",
      node,
      isClient(node) ? "client" : "container",
      label,
      node == rootNode    ? ",\"root\":\"true\""    : "",
      node == focusedNode ? ",\"focus\":\"true\""    : "",
      node == viewNode    ? ",\"view\":\"true\""    : "",
      node -> focus ? node -> focus : "",
      nestString
      );

  /* If it was a container, we malloc'd room for nestString */
  if (sizeof(nestString) > sizeof(char*))
    free(nestString);

  realloc(buffer, bytesUntilNull(buffer));
  return buffer;
}
