// Include necessary libraries
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"
#include "SPI.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include 
#include 
#include 

// WiFi Credentials
#define WIFI_SSID "realme 7"
#define WIFI_PASSWORD "asdfghjkl"

// SMTP Server Settings
#define SMTP_server "smtp.gmail.com"
#define SMTP_Port 465
#define sender_email "earthstower01@gmail.com"
#define sender_password "xfevtbiewmdxpcjl"
#define Recipient_email "earthsreceiver@gmail.com"
   
// Pin definition for onboard flash and indicator LEDs
#define FLASH_PIN 4
#define PICTURE_INDICATOR_LED 2
#define EMAIL_INDICATOR_LED 5
#define EMAIL_BLINK_PIN 12 // Define pin for blinking during email send

// Camera configuration for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;
SMTPSession smtp;
ESP_Mail_Session session;

// Global variable to track if the process has completed
bool operationCompleted = false;

// Variable to store the time when the operation completes
unsigned long operationCompleteTime = 0;

// Flag to control blinking
volatile bool isBlinking = false;

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
    Serial.begin(115200);
    
    // Initialize pin modes
    pinMode(FLASH_PIN, OUTPUT);
    pinMode(PICTURE_INDICATOR_LED, OUTPUT);
    pinMode(EMAIL_INDICATOR_LED, OUTPUT);
    pinMode(EMAIL_BLINK_PIN, OUTPUT); // Pin for blinking LED

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(200);
    }
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

    // SMTP session initialization
    smtp.debug(1);
    
    session.server.host_name = SMTP_server;
    session.server.port = SMTP_Port;
    session.login.email = sender_email;
    session.login.password = sender_password;

    // Load the picture number from EEPROM
    EEPROM.begin(1);
    pictureNumber = EEPROM.read(0);

    // Camera configuration
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA; 
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    
    // Start accessing SD card
    Serial.println("Start accessing SD Card 1-bit mode");
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD Card Mount Failed");
        return;
    }
    Serial.println("Started accessing SD Card 1-bit mode successfully");

    // Turn on the FLASH_PIN indefinitely
    digitalWrite(FLASH_PIN, HIGH); // Keep the flash LED on

    // Send email
    sendEmail();

    // Take pictures for 20 seconds
    takePicturesForDuration(20);

    // After taking pictures, turn off the flash LED
    digitalWrite(FLASH_PIN, LOW);
    
    // Mark the operation as completed
    operationCompleted = true;
    operationCompleteTime = millis(); // Store the time when operation completes
}

void takePicturesForDuration(int duration) {
    unsigned long startTime = millis();
    
    // Stop blinking when taking pictures
    isBlinking = false;

    while (millis() - startTime < duration * 1000) {
        // Start taking a picture
        digitalWrite(PICTURE_INDICATOR_LED, HIGH); // Indicate picture taking
        digitalWrite(FLASH_PIN, HIGH); // Turn on the LED Flash
        delay(1000); // Allow time for the flash to charge

        // Capture a picture
        camera_fb_t *fb = esp_camera_fb_get();  
        if (!fb) {
            Serial.println("Camera capture failed");
        } else {
            // Save image to MicroSD Card
            pictureNumber++;
            String path = "/picture" + String(pictureNumber) + ".jpg"; // Path where new picture will be saved
            File file = SD_MMC.open(path.c_str(), FILE_WRITE);
            if (!file) {
                Serial.println("Failed to open file in writing mode");
            } else {
                file.write(fb->buf, fb->len); // payload (image) and payload length
                Serial.printf("Saved file to path: %s\n", path.c_str());
                EEPROM.write(0, pictureNumber);
                EEPROM.commit();
            }
            file.close();
            esp_camera_fb_return(fb); // Return the frame buffer for reuse
            Serial.println("Picture taken and saved.");
        }

        // Turn off LED Flash and indicator LED
        digitalWrite(FLASH_PIN, LOW); 
        digitalWrite(PICTURE_INDICATOR_LED, LOW); // Turn off the picture indicator
        delay(1000); // Delay between pictures
    }
}

void sendEmail() {
    digitalWrite(EMAIL_INDICATOR_LED, HIGH); // Indicate email sending

    // Start blinking the email blink pin
    isBlinking = true;
    xTaskCreate(blinkEmailLED, "EmailBlinkTask", 1024, NULL, 1, NULL);

    // Send email with picture details
    SMTP_Message message;
    message.sender.name = "ESP32";
    message.sender.email = sender_email;
    message.subject = "EMERGENCY ALERT";
    message.addRecipient("", Recipient_email);

    // Email message content
    String textMsg = "Emergency in Tower 1: Dr. A. Santos Ave.";
    message.text.content = textMsg.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    if (!smtp.connect(&session)) {
        Serial.println("SMTP connection failed");
        isBlinking = false; // Stop blinking on failure
        digitalWrite(EMAIL_INDICATOR_LED, LOW); // Turn off email indicator
        return;
    }

    if (!MailClient.sendMail(&smtp, &message)) {
        Serial.println("Error sending Email: " + smtp.errorReason());
    } else {
        Serial.println("Email sent successfully.");
    }

    // Stop blinking after sending
    isBlinking = false; 
    digitalWrite(EMAIL_INDICATOR_LED, LOW); // Turn off email indicator
}

void blinkEmailLED(void * parameter) {
    while (isBlinking) {
        digitalWrite(EMAIL_BLINK_PIN, HIGH);
        delay(250);
        digitalWrite(EMAIL_BLINK_PIN, LOW);
        delay(250);
    }
    vTaskDelete(NULL); // Delete the task when finished
}

void loop() {
    // If the operation is completed, check for a reset after 1 minute
    if (operationCompleted) {
        if (millis() - operationCompleteTime >= 60000) { // 60000 milliseconds = 1 minute
            ESP.restart(); // Restart the ESP32 after 1 minute
        }
        return; // Do nothing else if operation is complete
    }
}