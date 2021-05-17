// RR_BLOCKING and RR_NON_BLOCKING
// inline void flatmap_function(data_t in, const uint idx, uint * w)

// KEYBY and BROADCAST
// inline void flatmap_function(data_t in, const uint idx)
inline void flatmap_function(data_t in, const uint idx, uint * w)
{
    int n = 0;
    bool flat_done = false;
    while (!flat_done) {

        send(in);

        if (n < 3) {
            n++;
        } else {
            flat_done = true;
        }
    }
}
