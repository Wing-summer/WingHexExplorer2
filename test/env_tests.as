// env_tests.as
// Test environment helpers (may depend on host env)
void main() {
    string val = env::readEnvironmentVar("PATH", "");
    println("PATH length: " + val.length());
    bool isset = env::isEnvironmentVarSet("PATH");
    println("is PATH set: " + isset);
    println("env_tests OK");
}
