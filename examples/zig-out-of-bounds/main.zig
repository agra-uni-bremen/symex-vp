const SymEx = @import("symex.zig").SymEx;
const StackTrace = @import("std").builtin.StackTrace;

// Global instantiation of the SymEX struct.
const CTRL = SymEx{};

// Implementation of exit, used from start.S.
export fn exit() void {
    CTRL.exit();
}

// Custom panic handler for zig panics.
pub fn panic(msg: []const u8, error_return_trace: ?*StackTrace) noreturn {
    CTRL.panic();

    // Make sure this function never returns
    while (true) {
        asm volatile("wfi\n");
    }
}

export fn main() void {
    const array = [_]u8{ 1, 2, 3, 4, 5 };

    var idx: u32 = 0;
    CTRL.make_symbolic(@ptrToInt(&idx), @sizeOf(u32));

    // This causes a panic if idx == 5.
    if (idx <= array.len) {
        _ = array[idx];
    }
}
