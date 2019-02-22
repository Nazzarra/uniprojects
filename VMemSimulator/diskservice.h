#ifndef _VM_DISK_SERVICE_H
#define _VM_DISK_SERVICE_H

#include <part.h>
#include "vm_declarations.h"
#include "pmt.h"

#include <stack>

class DiskService
{
public:
	DiskService(Partition* assignedPartition);
	~DiskService();

	ClusterNo getFreeCluster();
	void freeCluster(ClusterNo cluster);
	bool hasFreeClusters() const;
	ClusterNo getFreeClusterCount() const;

	void writePageToDisk(PageDescriptor& page);
	void readPageFromDisk(PageDescriptor& page);

private:
	ClusterNo freeClusterPointer, freeClusterCnt;
	std::stack<ClusterNo> releasedClusters;
	Partition* partition;
};

#endif