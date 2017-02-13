package main

import "sync"
import "C"

const (
	PORT_UNDEFINED = 0
	PORT_TELNET = 1
	PORT_BINARY = 2
	PORT_ASCII = 3
	PORT_MUD = 4
	PORT_WEBSOCKET =5
)

var (
	networkStat NetworkStats
	networkStatLock sync.Mutex // protect networkStat
)

type NetworkStats struct {
	inet_packets uint64
	inet_volume uint64
	inet_out_volume uint64
	inet_out_packets uint64

	portStats [5]PortStats
}

type PortStats struct {
	in_packets uint64
	in_volume uint64
	out_packets uint64
	out_volume uint64
}

func RecordIngress(idx int, n uint64) {
	networkStatLock.Lock()

	networkStat.inet_packets++
	networkStat.inet_volume += n
	networkStat.portStats[idx].in_packets++
	networkStat.portStats[idx].in_volume += n

	networkStatLock.Unlock()
}

func RecordEgress(idx int, n uint64) {
	networkStatLock.Lock()

	networkStat.inet_out_packets++
	networkStat.inet_out_volume += n
	networkStat.portStats[idx].out_packets++
	networkStat.portStats[idx].out_volume += n

	networkStatLock.Unlock()
}

// Go currently doesn't support exporting structs, so we are stuck exporting as
// multiple return values.
//export GetNetworkStats
func GetNetworkStats() (inet_packets uint64, inet_volume uint64, inet_out_packets uint64, inet_out_volume uint64) {
	networkStatLock.Lock()
	defer networkStatLock.Unlock()

	inet_packets = networkStat.inet_packets
	inet_volume = networkStat.inet_volume
	inet_out_packets = networkStat.inet_out_packets
	inet_out_volume = networkStat.inet_out_volume

	return
}
//export GetPortNetworkStats
func GetPortNetworkStats(port_idx int)(in_packets uint64, in_volume uint64, out_packets uint64, out_volume uint64) {
	networkStatLock.Lock()
	defer networkStatLock.Unlock()

	portStat := networkStat.portStats[port_idx]

	in_packets = portStat.in_packets
	in_volume = portStat.in_volume
	out_packets = portStat.out_packets
	out_volume = portStat.out_volume

	return
}