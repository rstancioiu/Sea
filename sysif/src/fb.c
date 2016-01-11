#include "fb.h"
#include "sched.h"
#include "syscall.h"

/*
 * Adresse du framebuffer, taille en byte, résolution de l'écran, pitch et depth (couleurs)
 */
static uint32 fb_address;
static uint32 fb_size_bytes;
static uint32 fb_x,fb_y,pitch,depth;

/*
 * Fonction pour lire et écrire dans les mailboxs
 */

/*
 * Fonction permettant d'écrire un message dans un canal de mailbox
 */
void MailboxWrite(uint32 message, uint32 mailbox)
{
  uint32 status;
  
  if(mailbox > 10)            // Il y a que 10 mailbox (0-9) sur raspberry pi
    return;
  
  // On attend que la mailbox soit disponible i.e. pas pleine
  do{
    /*
     * Permet de flusher
     */
    data_mem_barrier();
    status = mmio_read(MAILBOX_STATUS);
    
    /*
     * Permet de flusher
     */
    data_mem_barrier();
  }while(status & 0x80000000);             // Vérification si la mailbox est pleinne
  
  data_mem_barrier();
  mmio_write(MAILBOX_WRITE, message | mailbox);   // Combine le message à envoyer et le numéro du canal de la mailbox puis écrit en mémoire la combinaison
}


/*
 * Fonction permettant de lire un message et le retourner depuis un canal de mailbox
 */
uint32 MailboxRead(uint32 mailbox)
{
  uint32 status;
  
  if(mailbox > 10)             // Il y a que 10 mailbox (0-9) sur raspberry pi
    return 0;
  
  while(1){
    // On attend que la mailbox soit disponible pour la lecture, i.e. qu'elle n'est pas vide
    do{
      data_mem_barrier();
      status = mmio_read(MAILBOX_STATUS);
      data_mem_barrier();
    }while(status & 0x40000000);             // On vérifie que la mailbox n'est pas vide
    
    data_mem_barrier();
    status = mmio_read(MAILBOX_BASE);
    data_mem_barrier();
    
    // On conserve uniquement les données et on les retourne
    if(mailbox == (status & 0x0000000f))
      return status & 0x0000000f;
  }
}

/*
 * Fonction pour initialiser et écrire dans le framebuffer
 */

int FramebufferInitialize() {
  
  uint32 retval=0;
  volatile unsigned int mb[100] __attribute__ ((aligned(16)));
  
  depth = 24;
  
  //
  // Tout d'abord, on veut récupérer l'adresse en mémoire du framebuffer
  //
  mb[0] = 8 * 4;		// Taille du buffer i.e. de notre message à envoyer dans la mailbox
  mb[1] = 0;			// On spécifie qu'on demande quelque chose
  mb[2] = 0x00040003;	// La question que l'on pose: https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
  mb[3] = 2*4;		// La taille de la réponse
  mb[4] = 0;			// On indique que c'est une question ou un réponse (0 question)
  mb[5] = 0;			// Largeur
  mb[6] = 0;			// Hauteur
  mb[7] = 0;			// Marqueur de fin
  
  MailboxWrite((uint32)(mb+0x40000000), 8); // On écrit le message dans la mailbox
  
  if(((retval = MailboxRead(8)) == 0) || (mb[1] != 0x80000000)){ // On vérifie que le message est passé
    return 0;
  }
  
  fb_x = mb[5]; // On récupére la largeur en pixel de l'écran
  fb_y = mb[6]; // On récupére la hauteur en pixel de l'écran
  
  uint32 mb_pos=1;
  
  mb[mb_pos++] = 0;			// C'est une requête
  mb[mb_pos++] = 0x00048003;	// On définit la hauteur et la largeur du framebuffer
  mb[mb_pos++] = 2*4;			// On envoi 2 int pour la taille donc on spécifie la taille du buffer
  mb[mb_pos++] = 2*4;			// Taille du message (tag + indicateur de requête)
  mb[mb_pos++] = fb_x;		// On passe la largeur
  mb[mb_pos++] = fb_y;		// On passe la hauteur
  
  mb[mb_pos++] = 0x00048004;	// On définit la hauteur et la largeur virtuel du framebuffer
  mb[mb_pos++] = 2*4;			// On envoi 2 int pour la taille donc on spécifie la taille du buffer
  mb[mb_pos++] = 2*4;			// Taille du message (tag + indicateur de requête)
  mb[mb_pos++] = fb_x;		// On passe la largeur
  mb[mb_pos++] = fb_y;		// On passe la hauteur
  
  mb[mb_pos++] = 0x00048005;	// On définit la profondeur du frame buffer
  mb[mb_pos++] = 1*4;			
  mb[mb_pos++] = 1*4;			
  mb[mb_pos++] = depth;		// Profondeur i.e. nombre de couleur (24bit dans notre cas)
  
  mb[mb_pos++] = 0x00040001;	// On demande l'allocation du buffer
  mb[mb_pos++] = 2*4;			
  mb[mb_pos++] = 2*4;			
  mb[mb_pos++] = 16;			
  mb[mb_pos++] = 0;			

  mb[mb_pos++] = 0;			// Tag de fin de message
  mb[0] = mb_pos*4;			// Taille du message dans son entier
  
  MailboxWrite((uint32)(mb+0x40000000), 8); // On écrit dans la mailbox
  
  if(((retval = MailboxRead(8)) == 0) || (mb[1] != 0x80000000)){ // On vérifie que le message a bien été passé
    return 0;
  }
  
  /*
   * On récupére les différente information récupérer de la requête pour pouvoir reconstruire l'adresse du framebuffer et sa taille
   */
  mb_pos = 2;
  unsigned int val_buff_len=0;
  while(mb[mb_pos] != 0){
    switch(mb[mb_pos++])
    {
      case 0x00048003:
	val_buff_len = mb[mb_pos++];
	mb_pos+= (val_buff_len/4)+1;
	break;
      case 0x00048004:
	val_buff_len = mb[mb_pos++];
	mb_pos+= (val_buff_len/4)+1;
	break;
      case 0x00048005:
	val_buff_len = mb[mb_pos++];
	mb_pos+= (val_buff_len/4)+1;
	break;
      case 0x00040001:
	val_buff_len = mb[mb_pos++];
	mb_pos++;
	fb_address = mb[mb_pos++];
	fb_size_bytes = mb[mb_pos++];
	break;
    }
  }
  
  //
  // Récupére le pitch (This indicates the number of bytes between rows. Usually it will be related to the width, but there are exceptions such as when drawing only part of an image.)
  //
  mb[0] = 8 * 4;		// Taille du buffer
  mb[1] = 0;			// C'est une requête
  mb[2] = 0x00040008;	// On veut récupérer le pitch
  mb[3] = 4;			// Taille du buffer
  mb[4] = 0;			// Taille de la demande
  mb[5] = 0;			// Le pitch sera stocké ici
  mb[6] = 0;			// Tag de fin de message
  mb[7] = 0;
  
  MailboxWrite((uint32)(mb+0x40000000), 8);
  
  if(((retval = MailboxRead(8)) == 0) || (mb[1] != 0x80000000)){
    return 0;
  }
  
  pitch = mb[5];
  
  fb_x--;
  fb_y--;
  
  return 1;
}

/*
 * Fonction permettant de dessiner un pixel à l'adresse x,y avec la couleur rgb red.green.blue
 */
void put_pixel_RGB24(uint32 x, uint32 y, uint8 red, uint8 green, uint8 blue)
{
	volatile uint32 *ptr=0;
	uint32 offset=0;

	offset = (y * pitch) + (x * 3);
	ptr = (uint32*)(fb_address + offset);
	*((uint8*)ptr) = red;
	*((uint8*)(ptr+1)) = green;
	*((uint8*)(ptr+2)) = blue;
}

/*
 * Dessine sur tous les pixels des couleurs différentes
 */
void draw() {
  uint8 red=0,green=0,blue=0;
  uint32 x=0, y=0;
  for (x = 0; x < fb_x; x++) {
    for (y = 0; y < fb_y; y++) {
      if (blue > 254) {
	if (green > 254) {
	  if (red > 254) {
	    red=0; green=0; blue=0;
	  } else {
	    red++;
	  }
	} else {
	  green++;
	}
      } else blue++;
      put_pixel_RGB24(x,y,red,green,blue);
    }
  }
}

/*
 * Rempli l'écran de rouge
 */
void drawRed() {
  uint32 x=0, y=0;
  for (x = 0; x < fb_x; x++) {
    for (y = 0; y < fb_y; y++) {
      put_pixel_RGB24(x,y,255,0,0);
    }
  }
}

/*
 * Rempli l'écran de bleu
 */
void drawBlue() {
  uint32 x=0, y=0;
  for (x = 0; x < fb_x; x++) {
    for (y = 0; y < fb_y; y++) {
      put_pixel_RGB24(x,y,0,0,255);
    }
  }
}

/*
 * Rempli l'écran de vert
 */
void drawGreen() {
  uint32 x=0, y=0;
  for (x = 0; x < fb_x; x++) {
    for (y = 0; y < fb_y; y++) {
      put_pixel_RGB24(x,y,0,255,0);
    }
  }
}
