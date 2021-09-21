const SYMEX_CTRL_ADDR: usize = 0x02020000;
const SYMEX_CTRL_SIZE: usize = 0x02020004;
const SYMEX_CTRL_CTRL: usize = 0x02020008;

const SYMEX_ERROR: u32 = 1 << 31;
const SYMEX_EXIT:  u32 = 1 << 30;

pub const SymEx = struct {
    pub fn exit(self: SymEx) void {
        const ptr = @intToPtr(*volatile u32, SYMEX_CTRL_CTRL);
        ptr.* = SYMEX_EXIT;
    }

    pub fn panic(self: SymEx) void {
        const ptr = @intToPtr(*volatile u32, SYMEX_CTRL_CTRL);
        ptr.* = SYMEX_ERROR;
    }

    pub fn make_symbolic(self: SymEx, addr: usize, size: u32) void {
        const aptr = @intToPtr(*volatile u32, SYMEX_CTRL_ADDR);
        const sptr = @intToPtr(*volatile u32, SYMEX_CTRL_SIZE);

        aptr.* = addr;
        sptr.* = size;
    }
};
