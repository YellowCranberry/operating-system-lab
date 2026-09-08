#!/usr/bin/perl -w

# Emit the assembly entry points for each user syscall.

print "# generated syscall stubs; edit usys.pl instead\n";
print "#include \"kernel/syscall.h\"\n";

sub entry {
    my $prefix = "sys_";
    my $name = shift;
    if ($name eq "sbrk") {
	print ".global $prefix$name\n";
	print "$prefix$name:\n";
    } else {
	print ".global $name\n";
	print "$name:\n";
    }
    print " li a7, SYS_${name}\n";
    print " ecall\n";
    print " ret\n";
}
	
entry("fork");
entry("exit");
entry("wait");
entry("pipe");
entry("read");
entry("write");
entry("close");
entry("kill");
entry("exec");
entry("open");
entry("mknod");
entry("unlink");
entry("fstat");
entry("link");
entry("mkdir");
entry("chdir");
entry("dup");
entry("getpid");
entry("sbrk");
entry("pause");
entry("uptime");
entry("sync");
entry("shm_get");
entry("sem_init");
entry("sem_wait");
entry("sem_signal");
