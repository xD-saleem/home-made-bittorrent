#ifndef BITTORRENTCLIENT_PIECE
#define BITTORRENTCLIENT_PIECE

#include <string>
enum class BlockStatus { missing = 0, pending = 1, retrieved = 2 };

class Block {
 public:
  Block(int piece, int offset, int length, BlockStatus status,
        const std::string& data)
      : piece_(piece),
        offset_(offset),
        length_(length),
        status_(status),
        data_(data) {}

  int getPiece() const { return piece_; }
  void setPiece(int piece) { piece_ = piece; }

  int getOffset() const { return offset_; }
  void setOffset(int offset) { offset_ = offset; }

  int getLength() const { return length_; }
  void setLength(int length) { length_ = length; }

  BlockStatus getStatus() const { return status_; }
  void setStatus(BlockStatus status) { status_ = status; }

  const std::string& getData() const { return data_; }
  void setData(const std::string& data) { data_ = data; }

 private:
  int piece_;
  int offset_;
  int length_;
  BlockStatus status_;
  std::string data_;
};

#endif  // BITTORRENTCLIENT_BLOCK_H
