typedef struct __attribute__((packed, aligned(4))) {
    uint key;
    float value;
    float average;
    bool outlier;
} data_t;
