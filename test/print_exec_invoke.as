// print_exec.as
// Test exec, print variants
void main() {
    string rout;
    int rc = exec(rout, "echo", "hello", 1000);
    println("exec rc: " + rc + " outlen: " + rout.length());
    println("print_exec_invoke OK");
}
