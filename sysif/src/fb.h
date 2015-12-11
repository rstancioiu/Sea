#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "util.h"
#include "hw.h"

typedef uint32_t uint32;
typedef uint8_t uint8;

/*
 * Explication des adresses, offsets, channels, etc: http://elinux.org/RPi_Framebuffer
 * Intro au framebuffer: http://magicsmoke.co.za/?p=284
 */


/*
 * Mailbox functions
 */

enum {
    MAILBOX_BASE            = 0x2000B880,
    MAILBOX_POLL            = 0x2000B890,    // Receive without retrieving.	 R
    MAILBOX_SENDER          = 0x2000B894,    // Sender information.	 R
    MAILBOX_STATUS          = 0x2000B898,    // Information.	 R
    MAILBOX_CONFIGURATION   = 0x2000B89C,    // Settings.	 RW
    MAILBOX_WRITE           = 0x2000B8A0     // Sending mail.	 W
};

void MailboxWrite(uint32 message, uint32 mailbox);
uint32 MailboxRead(uint32 mailbox);

#define data_mem_barrier() __asm__ __volatile__ ("mcr p15, 0, %[reg], c7, c10, 5"::[reg] "r" (0))
#define data_sync_barrier() __asm__ __volatile__ ("mcr p15, 0, %[reg], c7, c10, 4"::[reg] "r" (0))


// fonction pour écrire un message data dans une mailbox suivant un des mode de l'enum si dessus 
static inline void mmio_write(uint32 reg, uint32 data) {
    uint32 *ptr = (uint32*)reg;
    __asm__ volatile("str %[data], [%[reg]]"
                 : : [reg]"r"(ptr), [data]"r"(data));
}


// fonction pour lire un message data dans une mailbox suivant un des mode de l'enum si dessus
static inline uint32 mmio_read(uint32 reg) {
    uint32 *ptr = (uint32*)reg;
    uint32 data;
    __asm__ volatile("ldr %[data], [%[reg]]"
                 : [data]"=r"(data) : [reg]"r"(ptr));
    return data;
}

/*
 * Framebuffer functions
 */

int FramebufferInitialize();

void draw();

void drawRed();

void drawBlue();

#endif
