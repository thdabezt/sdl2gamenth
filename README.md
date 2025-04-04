# Monster Shooter (Tên game dự kiến)

Một game bắn súng sinh tồn 2D từ trên xuống được xây dựng bằng C++ và SDL2, lấy cảm hứng từ các game như Vampire Survivors hoặc HoloCure. Người chơi điều khiển nhân vật, chiến đấu chống lại lũ quái vật, thu thập kinh nghiệm, lên cấp và nâng cấp sức mạnh để sống sót và đánh bại Boss.

## Tính năng chính

* **Lối chơi Sinh tồn:** Di chuyển bằng WASD, nhân vật tự động tấn công (vũ khí chính bắn theo chuột, phép thuật tự động kích hoạt).
* **Hệ thống ECS:** Xây dựng trên kiến trúc Entity-Component-System linh hoạt.
* **Chiến đấu:**
    * Vũ khí chính có thể nâng cấp (Damage, Fire Rate, Pierce, Projectiles, Burst...).
    * Nhiều loại phép thuật với cooldown và hiệu ứng riêng (Starfall, Fire Vortex...).
    * Va chạm AABB giữa đạn, người chơi, kẻ địch và địa hình.
* **Kẻ địch:**
    * Nhiều loại kẻ địch với AI cơ bản (đuổi theo người chơi).
    * Boss với AI phức tạp, nhiều trạng thái tấn công (Slam, Projectile Burst).
    * Hệ thống sinh sản quái vật theo level người chơi, có trọng số và nâng cấp.
* **Tiến trình & Nâng cấp:**
    * Hệ thống Level và Kinh nghiệm (nhặt EXP orb từ quái).
    * Hệ thống Nâng cấp Buff ngẫu nhiên khi lên cấp (tăng chỉ số, cải thiện vũ khí/phép).
    * Chỉ số Lifesteal.
* **Giao diện:** Hiển thị đầy đủ thông tin (HP, EXP, Level, Stats, Boss HP), Menu, Pause, Game Over, màn hình chọn Buff.
* **Âm thanh:** Nhạc nền và hiệu ứng âm thanh SFX (SDL\_mixer). Điều chỉnh âm lượng.
* **Lưu/Tải game:** Lưu và tải lại tiến trình chơi.
* **Bản đồ:** Tilemap với các vật cản. Camera theo dõi người chơi.
* **Fullscreen:** Hỗ trợ chuyển đổi chế độ cửa sổ/toàn màn hình (F11).

## Công nghệ sử dụng

* **Ngôn ngữ:** C++
* **Thư viện:**
    * SDL2 (Core, Image, TTF, Mixer)
* **Kiến trúc:** Entity-Component-System (ECS)

## Cấu trúc thư mục

* `src/`: Mã nguồn C++
    * `ECS/`: Các thành phần của hệ thống ECS (Entity, Component, Manager).
    * `Scene/`: Quản lý các màn chơi (Menu, Game).
* `sprites/`: Chứa các file hình ảnh (nhân vật, địch, map, đạn...).
* `assets/`: Chứa các tài nguyên khác (âm thanh, font, file map...).
* `saves/`: Thư mục chứa các file save game (tạo ra khi chạy).
* `build/`: Thư mục chứa các file object trung gian (tạo ra khi build).
* `makefile`: File để build project.

## Assets & Credits

* **Menu BGM:** [https://www.youtube.com/watch?v=vZQP5dB4uoA&list=PLL0CQjrcN8D0YyqI9uaxmoTRJuQ5IuZz7&index=10](https://www.youtube.com/watch?v=vZQP5dB4uoA&list=PLL0CQjrcN8D0YyqI9uaxmoTRJuQ5IuZz7&index=10) (Lưu ý: Link này có thể không truy cập được trực tiếp)
* **Game BGM:** [https://www.youtube.com/watch?v=IyvZh6fdqR0&list=PL6N1UyX9iZqs3FUvkgmm4_HB6ItIG6_T5&index=2](https://www.youtube.com/watch?v=IyvZh6fdqR0&list=PL6N1UyX9iZqs3FUvkgmm4_HB6ItIG6_T5&index=2) (Lưu ý: Link này có thể không truy cập được trực tiếp)
* **Game Over Video (Reference):** [https://motionarray.com/stock-video/game-over-on-retro-video-game-crt-screen-1921255/](https://motionarray.com/stock-video/game-over-on-retro-video-game-crt-screen-1921255/)
* **SFX - Click:** [https://pixabay.com/sound-effects/click-button-140881/](https://pixabay.com/sound-effects/click-button-140881/)
* **SFX - Game Over:** [https://pixabay.com/sound-effects/negative-beeps-6008/](https://pixabay.com/sound-effects/negative-beeps-6008/)
* **SFX - Fire (Edited):** [https://pixabay.com/sound-effects/basic-fire-whoosh-3-104223/](https://pixabay.com/sound-effects/basic-fire-whoosh-3-104223/)
* **SFX - Star:** [https://pixabay.com/sound-effects/cast-light-288736/](https://pixabay.com/sound-effects/cast-light-288736/)
* **Spritesheets:**
    * HoloCure Save the Fans!: [https://www.spriters-resource.com/pc_computer/holocuresavethefans/](https://www.spriters-resource.com/pc_computer/holocuresavethefans/)
    * Pokemon FireRed/LeafGreen (Map Tiles): [https://www.spriters-resource.com/game_boy_advance/pokemonfireredleafgreen/sheet/23610/](https://www.spriters-resource.com/game_boy_advance/pokemonfireredleafgreen/sheet/23610/)
    * Pokemon FireRed/LeafGreen (Map Tiles): [https://www.spriters-resource.com/game_boy_advance/pokemonfireredleafgreen/sheet/3735/](https://www.spriters-resource.com/game_boy_advance/pokemonfireredleafgreen/sheet/3735/)
* *(Lưu ý: Cần kiểm tra lại đường dẫn chính xác đến các file âm thanh/hình ảnh được sử dụng trong `constants.h` và `AssetManager` vì các link Pixabay/MotionArray chỉ là nguồn gốc).*

## Build & Chạy game

(Yêu cầu cài đặt môi trường build C++ với g++ và thư viện SDL2, SDL2\_image, SDL2\_ttf, SDL2\_mixer)

1.  **Build:** Mở terminal hoặc command prompt trong thư mục gốc của project và chạy:
    ```bash
    make all
    ```
2.  **Chạy game:**
    ```bash
    make run
    ```
    Hoặc chạy trực tiếp file thực thi (ví dụ: `./game` trên Linux/macOS hoặc `game.exe` trên Windows).
3.  **Clean:** Xóa các file build trung gian và file thực thi:
    ```bash
    make clean
    ```

*(Makefile được cung cấp có thể cần điều chỉnh đường dẫn đến thư viện SDL2 tùy thuộc vào hệ thống của bạn).*