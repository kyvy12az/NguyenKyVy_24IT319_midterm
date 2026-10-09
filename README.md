# Dự án giữa kỳ: Triển khai lệnh `ls(1)`

## 1. Giới thiệu

`myls` là phiên bản đơn giản hóa của lệnh UNIX `ls(1)`, được xây dựng bằng ngôn ngữ C dựa trên tài liệu hướng dẫn của NetBSD 10.1.

Chương trình thao tác trực tiếp với hệ thống tệp thông qua các API POSIX như:

- `opendir()`
- `readdir()`
- `stat()`
- `lstat()`
- `readlink()`
- `getopt()`

Chương trình hỗ trợ liệt kê tệp và thư mục, hiển thị metadata, sắp xếp kết quả, xử lý liên kết tượng trưng và duyệt thư mục đệ quy.

## 2. Tính năng đã triển khai

| Tùy chọn | Chức năng |
|---|---|
| `-A` | Hiển thị mục ẩn nhưng bỏ qua `.` và `..` |
| `-a` | Hiển thị tất cả mục, bao gồm `.` và `..` |
| `-c` | Sử dụng thời gian thay đổi trạng thái tệp |
| `-d` | Liệt kê chính thư mục thay vì mở và liệt kê nội dung |
| `-F` | Thêm ký hiệu phân loại `/`, `*`, `@`, `=` hoặc `\|` |
| `-f` | Không sắp xếp kết quả |
| `-h` | Hiển thị kích thước dễ đọc như `1.5K` hoặc `2M` |
| `-i` | Hiển thị số inode |
| `-k` | Hiển thị số block theo đơn vị 1 KiB |
| `-l` | Hiển thị thông tin chi tiết của tệp |
| `-n` | Giống `-l`, nhưng UID và GID được hiển thị ở dạng số |
| `-q` | Thay ký tự không in được bằng dấu `?` |
| `-R` | Liệt kê đệ quy các thư mục con |
| `-r` | Đảo ngược thứ tự sắp xếp |
| `-S` | Sắp xếp theo kích thước giảm dần |
| `-s` | Hiển thị số block thực tế đã sử dụng |
| `-t` | Sắp xếp theo thời gian, mới nhất xuất hiện trước |
| `-u` | Sử dụng thời gian truy cập gần nhất |
| `-w` | Giữ nguyên các ký tự không in được |

Ngoài các tùy chọn trên, chương trình còn hỗ trợ:

- Nhận nhiều toán hạng trong cùng một lệnh.
- Phân loại toán hạng thành tệp và thư mục.
- Hiển thị tệp trước thư mục.
- Hiển thị đích của liên kết tượng trưng trong chế độ định dạng dài.
- Đọc và xử lý biến môi trường `BLOCKSIZE`.
- Hiển thị các quyền đặc biệt như `setuid`, `setgid` và `sticky`.
- Tiếp tục xử lý các toán hạng còn lại khi một toán hạng gặp lỗi.
- Trả mã lỗi khác `0` khi có toán hạng không hợp lệ.

Các cặp tùy chọn đối lập sau áp dụng quy tắc: tùy chọn xuất hiện sau sẽ quyết định hành vi cuối cùng:

- `-l` và `-n`
- `-c` và `-u`
- `-R` và `-d`
- `-q` và `-w`
- `-h` và `-k`

## 3. Cấu trúc dự án

```text
NguyenKyVy_24IT319_midterm/
├── include/
│   ├── entry.h        # Khai báo cấu trúc mục và chức năng sắp xếp
│   ├── format.h       # Khai báo chức năng định dạng kết quả
│   ├── listing.h      # Khai báo chức năng liệt kê tệp và thư mục
│   ├── options.h      # Khai báo cấu hình và phân tích tùy chọn
│   └── util.h         # Khai báo các hàm tiện ích
├── src/
│   ├── entry.c
│   ├── format.c
│   ├── listing.c
│   ├── main.c
│   ├── options.c
│   └── util.c
├── tests/
│   └── test.sh        # Kịch bản kiểm thử tự động
├── .gitignore         # Loại trừ tệp nhị phân và tệp đối tượng
├── Makefile           # Biên dịch, kiểm thử và dọn dẹp dự án
└── README.md          # Tài liệu hướng dẫn
```

## 4. Yêu cầu môi trường

Chương trình đã được biên dịch và kiểm thử trên:

- NetBSD 10.1
- Trình biên dịch `cc` hỗ trợ chuẩn C11
- BSD Make
- Git để tải mã nguồn từ GitHub
- Kết nối Internet

Kiểm tra các công cụ cần thiết:

```sh
cc --version
make --version
git --version
```

Nếu NetBSD chưa có Git, có thể cài đặt bằng:

```sh
pkgin -y install git
```

## 5. Tải mã nguồn

Chuyển đến thư mục cá nhân của tài khoản hiện tại:

```sh
cd
```

Clone dự án từ GitHub:

```sh
git clone https://github.com/kyvy12az/NguyenKyVy_24IT319_midterm.git
```

Di chuyển vào thư mục dự án:

```sh
cd NguyenKyVy_24IT319_midterm
```

Kiểm tra các tệp trong dự án:

```sh
ls -la
```

## 6. Biên dịch chương trình

Chạy lệnh:

```sh
make
```

Nếu biên dịch thành công, tệp thực thi `myls` sẽ được tạo trong thư mục hiện tại.

Kiểm tra tệp thực thi:

```sh
ls -l myls
```

Biên dịch phiên bản có AddressSanitizer và UndefinedBehaviorSanitizer:

```sh
make debug
```

## 7. Chạy chương trình

Liệt kê nội dung thư mục hiện tại:

```sh
./myls
```

Hiển thị tất cả mục với thông tin chi tiết:

```sh
./myls -la
```

Hiển thị thư mục `/etc` theo định dạng dài với kích thước dễ đọc:

```sh
./myls -lh /etc
```

Liệt kê đệ quy thư mục `tests`:

```sh
./myls -R tests
```

Sắp xếp nội dung thư mục `/etc` theo kích thước giảm dần:

```sh
./myls -S /etc
```

Thêm ký hiệu phân loại vào sau tên tệp:

```sh
./myls -F
```

Có thể kết hợp nhiều tùy chọn trong cùng một lệnh:

```sh
./myls -kls
```

## 8. Kiểm thử

Chạy toàn bộ bộ kiểm thử tự động:

```sh
make test
```

Nếu chương trình hoạt động đúng, kết quả cuối cùng sẽ hiển thị:

```text
All tests passed.
```

Bộ kiểm thử tự động kiểm tra các chức năng:

- Ẩn tệp bắt đầu bằng dấu chấm trong chế độ mặc định.
- Hiển thị tệp ẩn với `-A` và `-a`.
- Phân loại thư mục, tệp thực thi và liên kết tượng trưng bằng `-F`.
- Hiển thị metadata và đích liên kết tượng trưng bằng `-l`.
- Duyệt thư mục đệ quy bằng `-R`.
- Liệt kê chính thư mục bằng `-d`.
- Xử lý quy tắc ghi đè giữa `-l` và `-n`.
- Sắp xếp theo kích thước bằng `-S`.
- Đảo ngược thứ tự bằng `-r`.
- Hiển thị inode, block và kích thước dễ đọc.
- Thay ký tự không in được bằng dấu `?`.
- Trả mã lỗi khi toán hạng không tồn tại.

## 9. Dọn sản phẩm biên dịch

Để xóa chương trình `myls` và toàn bộ tệp đối tượng `.o`, chạy:

```sh
make clean
```

Lệnh này chỉ xóa các sản phẩm được tạo ra trong quá trình biên dịch, không xóa mã nguồn.

## 10. Luồng xử lý chính

1. `options_parse()` sử dụng `getopt()` để đọc các tùy chọn dòng lệnh và tạo cấu hình dùng chung.
2. `list_operands()` gọi `lstat()` hoặc `stat()` để phân loại toán hạng thành tệp và thư mục.
3. Các tệp được hiển thị trước, sau đó chương trình xử lý từng thư mục.
4. Mỗi thư mục được mở bằng `opendir()` và đọc bằng `readdir()`.
5. Metadata của từng mục được lấy bằng `lstat()` để không vô tình đi theo liên kết tượng trưng.
6. Danh sách được sắp xếp theo tên, kích thước hoặc thời gian tùy theo các tùy chọn đã chọn.
7. Kết quả được chuyển đến mô-đun `format` để định dạng và in ra màn hình.
8. Khi có `-R`, chương trình tiếp tục duyệt các thư mục con và luôn bỏ qua `.` cùng `..` để tránh vòng lặp.

Các lỗi như đường dẫn không tồn tại, không đủ quyền mở thư mục hoặc không đọc được metadata sẽ được ghi ra `stderr`.

Chương trình vẫn tiếp tục xử lý các toán hạng còn lại và trả về:

- Mã `0`: chương trình hoàn thành thành công.
- Mã `1`: xảy ra ít nhất một lỗi trong quá trình xử lý.
- Mã `2`: lỗi cú pháp, tùy chọn không hợp lệ hoặc không đủ bộ nhớ.

## 11. Phân công mô-đun

| Mô-đun | Trách nhiệm chính |
|---|---|
| `options` | Khởi tạo cấu hình và xử lý tham số dòng lệnh |
| `entry` | Lưu metadata, quản lý mảng động và sắp xếp |
| `format` | Định dạng quyền, thời gian, kích thước, chủ sở hữu và tên tệp |
| `listing` | Đọc thư mục, phân loại toán hạng và duyệt đệ quy |
| `util` | Nối đường dẫn, sao chép chuỗi và xử lý `BLOCKSIZE` |

## 12. Lưu ý và giới hạn

- Chương trình chỉ triển khai các tùy chọn thuộc phạm vi tài liệu được cung cấp.
- Ký hiệu whiteout `%` phụ thuộc vào hệ thống tệp NetBSD. Những nền tảng không hỗ trợ kiểu tệp này sẽ không thể tạo dữ liệu thực tế để kiểm thử.
- Định dạng ngày và giờ phụ thuộc vào múi giờ cùng locale hiện tại của hệ thống.
- Mỗi mục được in trên một dòng theo hành vi mặc định được yêu cầu.
- Kết quả có thể khác đôi chút giữa NetBSD và các hệ điều hành UNIX/Unix-like khác.

## 13. Kho lưu trữ GitHub

Mã nguồn đầy đủ của dự án được lưu trữ tại:

[https://github.com/kyvy12az/NguyenKyVy_24IT319_midterm](https://github.com/kyvy12az/NguyenKyVy_24IT319_midterm)

## 14. Tác giả

- **Họ và tên:** Nguyễn Kỳ Vỹ
- **Mã sinh viên:** 24IT319
- **Lớp:** 24JIT
- **Trường:** Trường Đại học Công nghệ Thông tin và Truyền thông Việt – Hàn