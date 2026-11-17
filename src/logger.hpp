#pragma once

#include <cstdio>

class BufferLogger {
public:
    explicit BufferLogger(const char *path);
    ~BufferLogger();

    BufferLogger(const BufferLogger &) = delete;
    BufferLogger &operator=(const BufferLogger &) = delete;

    void log(size_t bufferIndex, int samples, int flags, long long timeNs, bool burst);

private:
    std::FILE *file_ = nullptr;
};