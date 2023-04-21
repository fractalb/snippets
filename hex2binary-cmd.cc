#include <fstream>
#include <iostream>
#include <stdexcept>

extern "C" {
#include "hex2binary.c"
}

class Buffer {
private:
  char *buf;
  size_t size;
  static const size_t init_size;

public:
  Buffer();
  Buffer(size_t size);
  ~Buffer();

  Buffer &operator=(const Buffer &b) = delete;
  void resizeBuf(size_t newSize);
  void ensureBufSize(size_t size);
  inline char *getBuf() { return buf; }
  inline size_t getSize() { return size; }
};

const size_t Buffer::init_size = 32;
Buffer::Buffer() : Buffer(init_size) {}

Buffer::Buffer(size_t size) : buf(nullptr), size(size) {
  buf = (char *)malloc(size);
  if (buf == nullptr) {
    throw std::bad_alloc();
  }
}

Buffer::~Buffer() {
  free(buf);
  size = 0;
}

void Buffer::resizeBuf(size_t newSize) {
  char *buf2 = (char *)malloc(newSize);
  if (buf2 == nullptr)
    throw std::bad_alloc();
  free(buf);
  buf = buf2;
  size = newSize;
}

void Buffer::ensureBufSize(size_t size) {
  if (size <= this->size)
    return;

  resizeBuf(size);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "File name is needed\n";
    return 0;
  }
  std::ifstream ifs(argv[1], std::ifstream::in);
  if (!ifs) {
    std::cerr << "Error opening file " << argv[1] << '\n';
    return EINVAL;
  }
  int error;
  Buffer buf;
  std::string line;
  std::getline(ifs, line);
  while (ifs) {
    buf.ensureBufSize(line.size());
    size_t s = decode_hex_to_binary(line.c_str(), line.size(), buf.getBuf(),
                                    buf.getSize(), &error);
    std::cerr << "decoded bytes s = " << s << "\n";
    if (error == -1) {
      std::cerr << "Error decoding hex string: " << line << '\n';
    }
    for (size_t i = 0; i < s; ++i) {
      std::cout << buf.getBuf()[i];
    }
    std::getline(ifs, line);
  }
  return 0;
}
