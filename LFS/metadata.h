/*
 * metadata.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef METADATA_H_
#define METADATA_H_


typedef struct
{
	char* consistency;
	int partitions;
	int compaction_time;
} tableMetadata;



#endif /* METADATA_H_ */
