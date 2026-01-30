#include <iostream>
#include <string>

// Lớp cơ sở trừu tượng đại diện cho một thiết bị chung
class Device {
protected:
    std::string brand;       // Thương hiệu của thiết bị
    int batteryLevel;        // Mức pin (tính theo phần trăm)

public:
    // Constructor khởi tạo thương hiệu và mức pin (dùng initializer list)
    Device(std::string b, int bat) : brand(b), batteryLevel(bat) {}

    // Hàm ảo có thể được ghi đè để thực hiện hành vi bật thiết bị chung
    virtual void turnOn() {
        std::cout << "Device is starting..." << std::endl;
    }

    // Hàm ảo thuần túy: lớp con bắt buộc phải triển khai showStatus()
    virtual void showStatus() = 0;
};

// Lớp con đại diện cho smartphone, kế thừa từ Device
class Smartphone : public Device {
private:
    std::string os; // Hệ điều hành của smartphone

public:
    // Constructor: truyền tham số lên lớp cơ sở và khởi tạo hệ điều hành
    Smartphone(std::string b, int bat, std::string system)
        : Device(b, bat), os(system) {}

    // Ghi đè phương thức turnOn để hiển thị thông tin khởi động cụ thể cho smartphone
    void turnOn() override {
        std::cout << "Smartphone " << brand << " booting " << os << "..." << std::endl;
    }

    // Phương thức riêng của Smartphone để thực hiện cuộc gọi
    void makeCall(std::string number) {
        std::cout << "Calling " << number << "..." << std::endl;
    }

    // Triển khai hàm ảo thuần túy từ lớp Device để hiển thị trạng thái thiết bị
    void showStatus() override {
        std::cout << "Brand: " << brand << ", Battery: " << batteryLevel << "%, OS: " << os << std::endl;
    }
};