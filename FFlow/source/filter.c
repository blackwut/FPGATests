inline data_t filter_function(data_t in)
{
    data_t out = in;
    if (out.key > 10) {
        out.value = 0.0f;
    }

    return out;
}