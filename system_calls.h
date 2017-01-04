#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include "types.h"				//use data types from here
#include "lib.h"				//use putc in order to write characters to the screen
#include "devices.h"
#include "pcb.h"
#include "file_system.h"
#include "paging.h"
#include "x86_desc.h"
#include "int_handler.h"
#include "scheduler.h"


int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open ();
int32_t rtc_close (int32_t fd);

int32_t file_read (int32_t fd, void* buf, int32_t nbytes);
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t file_open (const uint8_t* filename);
int32_t file_close (int32_t fd);

int32_t myprint(const void* buf);

int32_t term_read(int32_t fd, void* buf, int32_t nbytes);
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t term_open(const uint8_t* filename);
int32_t term_close(int32_t fd);

//**********SYSTEM CALLS FOR CHECKPOINT 3***********

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);

#endif