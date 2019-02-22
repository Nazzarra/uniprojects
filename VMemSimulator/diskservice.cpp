#include "diskservice.h"
#include "framepool.h"

DiskService::DiskService(Partition* assignedPartition)
	: freeClusterPointer(0), freeClusterCnt(assignedPartition->getNumOfClusters()), partition(assignedPartition)
{
}

DiskService::~DiskService()
{
}

ClusterNo DiskService::getFreeCluster()
{
	if (freeClusterCnt == 0)
		return ~(0LU);

	--freeClusterCnt;
	ClusterNo cluster;

	if (!releasedClusters.empty()) {
		cluster = releasedClusters.top();
		releasedClusters.pop();
	}
	else {
		cluster = freeClusterPointer;
		freeClusterPointer = (freeClusterPointer + 1) % partition->getNumOfClusters();
	}

	return cluster;
}

void DiskService::freeCluster(ClusterNo cluster)
{
	++freeClusterCnt;
	releasedClusters.push(cluster);
}

void DiskService::writePageToDisk(PageDescriptor& pg)
{
	partition->writeCluster(pg.cluster, (const char*)(pg.frame->frameStart));
}

void DiskService::readPageFromDisk(PageDescriptor& pg)
{
	partition->readCluster(pg.cluster, (char*)(pg.frame->frameStart));
}

bool DiskService::hasFreeClusters() const
{
	return freeClusterCnt != 0;
}

ClusterNo DiskService::getFreeClusterCount() const 
{
	return freeClusterCnt;
}