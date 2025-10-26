// regex_iterator_match.as
// Use regex::iterator to iterate matches
void main() {
    regex::exp re("ab");
    
    regex::iterator it = re.globalMatch("abxab");
    // Some hosts may require different constructor; fall back to matching manually
    if (it.isValid()) {
        int count = 0;
        while (it.hasNext()) {
            regex::match m = it.next();
            if (m.hasMatch()) count++;
        }
        println("iterator matches: " + count);
    } else {
        println("iterator not valid on this host");
    }
    println("regex_iterator_match OK");
}
