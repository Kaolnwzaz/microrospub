#include <WiFi.h>
#include <micro_ros_arduino.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int8.h>

// ---------------------- CONFIG ----------------------
#define WIFI_SSID      "YOUR_WIFI"
#define WIFI_PASSWORD  "YOUR_PASS"
#define AGENT_IP       "192.168.1.100"
#define AGENT_PORT     8888

// ---------------------- LED PINS --------------------
#define RED_LED_PIN     15
#define YELLOW_LED_PIN   3
#define GREEN_LED_PIN    1

// ---------------------- ROS VARIABLES ----------------
rcl_subscription_t LEDs_subscriber;
std_msgs__msg__Int8 LEDs_msg;

rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rclc_executor_t executor;

// ------------------- LED CALLBACK --------------------
void LEDs_subscription_callback(const void *msgin) {
  const std_msgs__msg__Int8 *msg = (const std_msgs__msg__Int8 *)msgin;
  int8_t value = msg->data;

  // reset LEDs
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  switch (value) {
    case 1:
      digitalWrite(RED_LED_PIN, HIGH);
      break;
    case 2:
      digitalWrite(YELLOW_LED_PIN, HIGH);
      break;
    case 3:
      digitalWrite(GREEN_LED_PIN, HIGH);
      break;
    case 4:   // all
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(YELLOW_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, HIGH);
      break;
    default:
      // value = 0 or invalid → all off
      break;
  }
}

// ------------------------- SETUP ----------------------
void setup() {
  Serial.begin(115200);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  set_microros_wifi_transports(
    (char*)WIFI_SSID,
    (char*)WIFI_PASSWORD,
    (char*)AGENT_IP,
    AGENT_PORT
  );

  allocator = rcl_get_default_allocator();

  // init micro-ROS
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "esp32_led_node", "", &support);

  // create subscriber
  rclc_subscription_init_default(
      &LEDs_subscriber,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int8),
      "LEDs");

  // executor
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_subscription(&executor, &LEDs_subscriber, &LEDs_msg,
                                 &LEDs_subscription_callback, ON_NEW_DATA);

  Serial.println("ESP32 micro-ROS LED Subscriber Ready!");
}

// -------------------------- LOOP -----------------------
void loop() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
}
