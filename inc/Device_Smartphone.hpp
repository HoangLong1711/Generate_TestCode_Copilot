#ifndef DEVICE_SMARTPHONE_H
#define DEVICE_SMARTPHONE_H

#include <iostream>
#include <string>

// Lớp cơ sở trừu tượng đại diện cho một thiết bị chung
class Device {
protected:
    std::string brand;      // Thương hiệu
    int batteryLevel;       // Mức pin

public:
    // Constructor device với thương hiệu và mức pin
    Device(std::string b, int bat);

    // Hàm ảo có thể được ghi đè để thực hiện hành vi bật thiết bị chung
    virtual void turnOn();

    // Hàm ảo thuần túy: lớp con bắt buộc phải triển khai showStatus()
    virtual void showStatus() = 0;

    // Virtual destructor để đảm bảo hủy đúng đối tượng khi dùng con trỏ đa hình
    virtual ~Device() = default;
};

// Lớp con đại diện cho smartphone, kế thừa từ Device
class Smartphone : public Device {
private:
    std::string os; // Hệ điều hành của smartphone

public:
    // Constructor: truyền tham số lên lớp cơ sở và khởi tạo os
    Smartphone(std::string b, int bat, std::string system);

    // Ghi đè phương thức turnOn để hiển thị thông tin khởi động cụ thể cho smartphone
    void turnOn() override;

    // Phương thức riêng của Smartphone để thực hiện cuộc gọi
    void makeCall(std::string number);

    // Triển khai hàm ảo thuần túy từ lớp Device để hiển thị trạng thái thiết bị
    void showStatus() override;
};

#endif // DEVICE_SMARTPHONE_H