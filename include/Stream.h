#ifndef STREAM_H
#define STREAM_H

struct StreamStruct;
typedef struct StreamStruct *Stream;

Stream Stream_Create(const char *filename);
int Stream_GetChunkSize(Stream);
void Stream_SetChunkSize(Stream, int);

#endif
