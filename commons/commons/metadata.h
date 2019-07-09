/*
 * metadata.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef COMMONS_METADATA_H_
#define COMMONS_METADATA_H_


typedef struct
{
	char* nombreTabla;
	char* consistency;
	int partitions;
	int compaction_time;
} metadataTabla;



#endif /* COMMONS_METADATA_H_ */
