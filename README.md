# Dự án giữa kỳ: triển khai `ls(1)`

## 1. Giới thiệu

`myls` là phiên bản đơn giản hóa của lệnh UNIX `ls(1)`, được viết bằng C dựa
trên trang hướng dẫn NetBSD 10.1 do giảng viên cung cấp. Chương trình thao tác
trực tiếp với hệ thống tệp thông qua `opendir`, `readdir`, `stat`, `lstat`,
`readlink` và các API POSIX liên quan.

## 2. Tính năng đã triển khai

| Tùy chọn | Chức năng |
|---|---|
| `-A` | Hiện mục ẩn, trừ `.` và `..` |
| `-a` | Hiện tất cả mục, kể cả `.` và `..` |
| `-c` | Dùng thời gian thay đổi trạng thái tệp |
| `-d` | Liệt kê chính thư mục, không mở nội dung |
| `-F` | Thêm ký hiệu phân loại `/`, `*`, `@`, `=`, `|` |
| `-f` | Không sắp xếp |
| `-h` | Kích thước dễ đọc như `1.5K`, `2M` |
| `-i` | Hiện số inode |
| `-k` | Hiện số block theo đơn vị 1 KiB |
| `-l` | Định dạng chi tiết |
| `-n` | Như `-l`, nhưng UID/GID ở dạng số |
| `-q` | Thay ký tự không in được bằng `?` |
| `-R` | Liệt kê đệ quy thư mục con |
| `-r` | Đảo ngược thứ tự sắp xếp |
| `-S` | Sắp xếp theo kích thước giảm dần |
| `-s` | Hiện số block thực tế đã dùng |
| `-t` | Sắp xếp theo thời gian mới nhất trước |
| `-u` | Dùng thời gian truy cập gần nhất |
| `-w` | In nguyên trạng ký tự không in được |

Chương trình còn hỗ trợ nhiều toán hạng, tách tệp và thư mục, hiển thị đích của
liên kết tượng trưng, biến môi trường `BLOCKSIZE`, quyền đặc biệt setuid,
setgid/sticky và trả mã lỗi khác 0 khi có toán hạng không hợp lệ.

Các cặp `-l/-n`, `-c/-u`, `-R/-d`, `-q/-w` và `-h/-k` áp dụng quy tắc tùy chọn
được viết sau sẽ quyết định hành vi cuối cùng.

## 3. Cấu trúc dự án

```text
ls-midterm/
├── include/
│   ├── entry.h       # Cấu trúc mục và sắp xếp
│   ├── format.h      # Định dạng kết quả
│   ├── listing.h     # Điều phối liệt kê tệp/thư mục
│   ├── options.h     # Cấu hình và phân tích tùy chọn
│   └── util.h        # Hàm tiện ích
├── src/
│   ├── entry.c
│   ├── format.c
│   ├── listing.c
│   ├── main.c
│   ├── options.c
│   └── util.c
├── tests/test.sh     # Kiểm thử tự động
├── Makefile
└── README.md
```

## 4. Biên dịch và chạy

Yêu cầu: hệ điều hành UNIX/Unix-like, trình biên dịch C và `make`.

```bash
make
./myls
./myls -la
./myls -lh /etc
./myls -R path/to/directory
```

Để biên dịch bản có AddressSanitizer và UndefinedBehaviorSanitizer:

```bash
make debug
```

Xóa sản phẩm biên dịch:

```bash
make clean
```

## 5. Kiểm thử

```bash
make test
```

Bộ kiểm thử tạo một thư mục tạm và kiểm tra: tệp ẩn, phân loại loại tệp, định
dạng dài, liên kết tượng trưng, đệ quy, `-d` và mã thoát khi gặp đường dẫn lỗi.

## 6. Luồng xử lý chính

1. `options_parse()` đọc tùy chọn bằng `getopt()` và tạo cấu hình dùng chung.
2. `list_operands()` gọi `lstat()`/`stat()` để tách toán hạng thành tệp và thư
   mục; tệp luôn được xuất trước theo yêu cầu của manual.
3. Mỗi thư mục được đọc bằng `opendir()` và `readdir()`. Metadata được lấy bằng
   `lstat()` để không vô tình đi theo liên kết tượng trưng.
4. Danh sách được sắp xếp theo tên, kích thước hoặc thời gian rồi chuyển sang
   mô-đun `format` để in.
5. Với `-R`, chỉ các mục thực sự có kiểu thư mục mới được duyệt tiếp; `.` và
   `..` luôn bị loại khỏi bước đệ quy để tránh vòng lặp.

Các lỗi như đường dẫn không tồn tại, không đủ quyền mở thư mục hoặc lỗi đọc
metadata được ghi ra `stderr`. Chương trình vẫn tiếp tục với các toán hạng còn
lại và trả mã `1` khi đã xảy ra ít nhất một lỗi. Lỗi cú pháp hoặc thiếu bộ nhớ
trả mã `2`.

## 7. Lưu ý và giới hạn

- Chương trình chỉ triển khai các tùy chọn có trong tài liệu được cung cấp.
- Ký hiệu whiteout `%` phụ thuộc hệ thống tệp NetBSD; nền tảng không cung cấp
  kiểu tệp này sẽ không thể tạo dữ liệu thực tế để kiểm thử.
- Định dạng ngày giờ sử dụng múi giờ và locale hiện tại của hệ thống.
- Mỗi mục được in trên một dòng, đúng với hành vi mặc định mô tả trong tài liệu.

## 8. Phân công mô-đun

- `options`: khởi tạo và xử lý tham số dòng lệnh.
- `entry`: lưu trữ metadata, quản lý mảng động và sắp xếp.
- `format`: định dạng quyền, thời gian, kích thước, chủ sở hữu và tên tệp.
- `listing`: đọc thư mục, phân loại toán hạng và duyệt đệ quy.
- `util`: nối đường dẫn, sao chép chuỗi và xử lý `BLOCKSIZE`.
