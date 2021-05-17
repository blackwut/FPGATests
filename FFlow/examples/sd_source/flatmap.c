int n = 0;
bool flat_done = false;
while (!flat_done) {

    send(in);

    if (n < 10) {
        n++;
    } else {
        flat_done = true;
    }
}
