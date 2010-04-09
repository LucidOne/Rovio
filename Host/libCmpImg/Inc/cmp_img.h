#ifndef __CMP_IMG_H__
#define __CMP_IMG_H__

typedef enum
{
	IMG_CMP_SENSIBILITY_HIGH	= 1,
	IMG_CMP_SENSIBILITY_MIDDLE	= 2,
	IMG_CMP_SENSIBILITY_LOW		= 3
} IMG_CMP_SENSIBILITY_E;

/*
 * width:	width of the images
 * heigh:	height of the images
 * bufp1:	buffer for first image, the size of buffer should be width * height
 * bufp2:	buffer for second image, the size of buffer should be width * height
 * blksize:	block size for compare. Here's some suggest value:
 * 				size 80*60 => 8
 * 				size 44*36 => 6
 * 				size 40*30 => 5
 * 				size 22*18 => 4
 */
int compare_image(
	unsigned int width,
	unsigned int height,
	const unsigned char *bufp1,
	const unsigned char *bufp2,
	int blksize,
	IMG_CMP_SENSIBILITY_E sensibility);

#endif

