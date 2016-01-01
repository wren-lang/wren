import "io" for File, Stat
import "scheduler" for Scheduler

var stat = File.stat("test/io/directory/dir")

System.print(stat is Stat)            // expect: true
System.print(stat.device is Num)      // expect: true
System.print(stat.inode is Num)       // expect: true
System.print(stat.mode is Num)        // expect: true
System.print(stat.linkCount)          // expect: 5
System.print(stat.user is Num)        // expect: true
System.print(stat.group is Num)       // expect: true
System.print(stat.specialDevice)      // expect: 0
System.print(stat.size)               // expect: 170
System.print(stat.blockSize is Num)   // expect: true
System.print(stat.blockCount is Num)  // expect: true
