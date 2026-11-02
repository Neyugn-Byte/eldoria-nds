ELDORIA - PROTOTYPE V0.1
========================

MỤC TIÊU
--------
Prototype homebrew Nintendo DS: bản đồ rừng 2D top-down, nhân vật,
va chạm cơ bản, Goblin đuổi theo, tấn công, HP/EXP/Gold và màn hình thông tin.

TÌNH TRẠNG
----------
- Đây là MÃ NGUỒN, chưa phải ROM NDS đã biên dịch.
- Gói này bổ sung GitHub Actions để biên dịch trên máy chủ đám mây.
- Bạn KHÔNG cần cài devkitPro, compiler, IDE hay phần mềm lập trình trên Windows.
- Cần có tài khoản GitHub và dùng trình duyệt để tải dự án lên GitHub.
- Khi workflow chạy thành công, GitHub tạo artifact có file .nds để tải về.
- Tôi chưa thể tuyên bố ROM đã chạy ổn trên melonDS/DSi cho đến khi có bản build
  thực tế và được kiểm tra trên emulator/thiết bị.

CẤU TRÚC
--------
source/main.c                  Mã nguồn game
Makefile                       Cấu hình build devkitPro/libnds
.github/workflows/build.yml    Tự động build trên GitHub Actions
GDD_VI.txt                     Tóm tắt thiết kế dự án

CÁCH LẤY FILE NDS MÀ KHÔNG CÀI PHẦN MỀM
---------------------------------------
1. Trên Windows, mở https://github.com và đăng nhập/tạo tài khoản miễn phí.
2. Nhấn dấu + ở góc trên bên phải, chọn New repository.
3. Đặt tên repository: eldoria-nds (có thể để Private), rồi chọn Create repository.
4. Trong repository mới, chọn Add file > Upload files.
5. Giải nén ZIP này trên máy tính. Mở thư mục eldoria_v01.
6. Upload TOÀN BỘ nội dung bên trong thư mục eldoria_v01, bao gồm thư mục .github.
   Nếu không thấy .github, bật hiển thị Hidden items trong File Explorer.
7. Chọn Commit changes.
8. Mở tab Actions. Chọn workflow "Build Eldoria NDS". Nếu workflow chưa tự chạy,
   chọn Run workflow.
9. Chờ workflow kết thúc. Nếu có dấu xanh, mở lần chạy đó.
10. Ở phần Artifacts, tải "Eldoria-V0.1-NDS-and-build.zip".
11. Giải nén artifact; file eldoria_v01.nds nằm bên trong. Mở bằng melonDS.

LƯU Ý VỀ GITHUB UPLOAD
---------------------
GitHub web uploader có thể không hiện thư mục ẩn .github khi kéo thả.
Nếu workflow không xuất hiện trong tab Actions, nguyên nhân thường là thư mục
.github/workflows/build.yml chưa được upload. Hãy bật View > Show > Hidden items
trong Windows File Explorer rồi upload lại, hoặc tạo workflow trực tiếp trên GitHub.

ĐIỀU KHIỂN DỰ KIẾN
------------------
D-pad: di chuyển
A: tấn công
START: bật/tắt dòng trợ giúp

GHI CHÚ
-------
- Hình ảnh hiện được vẽ bằng code, chưa phải sprite sheet nghệ thuật hoàn chỉnh.
- Đây là prototype kỹ thuật, chưa phải game hoàn chỉnh.
- Sau khi có ROM, cần kiểm tra khởi động, hình ảnh, điều khiển, va chạm, AI,
  hiệu năng và lưu dữ liệu. Bản V0.1 hiện chưa có hệ thống save/load.
- Không đổi tên file ELF thành .nds; điều đó không tạo ROM hợp lệ.
