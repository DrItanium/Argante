{ "generic", 0x1, 2 },
{ "outside_code", 0x2, 2 },
{ "outside_mem", 0x3, 2 },
{ "corrupt_code", 0x4, 2 },
{ "protfault", 0x5, 2 },
{ "mstack_over", 0x6, 2 },
{ "mstack_under", 0x7, 2 },
{ "oom", 0x8, 2 },
{ "cstack_over", 0x9, 2 },
{ "cstack_under", 0xa, 2 },
{ "nosyscall", 0xb, 2 },
{ "noperm", 0xc /* hac perm */, 2 },
{ "result_toolong", 0xd, 2 },
{ "bad_fd", 0xe, 2 },
{ "toomany_fds", 0xf, 2 },
{ "file_exists", 0x10, 2 },
{ "file_not_exist", 0x11, 2 },
{ "arg_toolong", 0x12, 2 },
{ "internal", 0x13, 2 },
{ "matherror", 0x14, 2 },
{ "alib_fail", 0x1000, 2 },
{ "alib_nosym", 0x1001, 2 },
{ "strfd_bounds", 0x1010, 2 },
{ "strfd_searchfail", 0x1011, 2 },