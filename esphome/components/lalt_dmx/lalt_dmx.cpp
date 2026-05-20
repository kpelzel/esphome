#include "lalt_dmx.h"
// #include "e131_light_effect_base.h"
#include "esphome/core/log.h"
#include "esp_dmx.h"


namespace esphome {
namespace lalt_dmx {

static const char *const TAG = "lalt_dmx";
static const int PORT = 5568;
static const int tickerJoin = 1000;
static const int TX_PIN = 17;
static const int RX_PIN = 5;
static const int EN_PIN = 15;
int ticker = 0;

LALTDMXComponent::LALTDMXComponent() {}

LALTDMXComponent::~LALTDMXComponent() {
  if (this->socket_) {
    this->socket_->close();
  }
}

void LALTDMXComponent::setup() {
  this->socket_ = socket::socket_ip(SOCK_DGRAM, IPPROTO_IP);

  int enable = 1;
  int err = this->socket_->setsockopt(SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
  if (err != 0) {
    ESP_LOGW(TAG, "Socket unable to set reuseaddr: errno %d", err);
    // we can still continue
  }
  err = this->socket_->setblocking(false);
  if (err != 0) {
    ESP_LOGW(TAG, "Socket unable to set nonblocking mode: errno %d", err);
    this->mark_failed();
    return;
  }

  struct sockaddr_storage server;

  socklen_t sl = socket::set_sockaddr_any((struct sockaddr *) &server, sizeof(server), PORT);
  if (sl == 0) {
    ESP_LOGW(TAG, "Socket unable to set sockaddr: errno %d", errno);
    this->mark_failed();
    return;
  }

  err = this->socket_->bind((struct sockaddr *) &server, sizeof(server));
  if (err != 0) {
    ESP_LOGW(TAG, "Socket unable to bind: errno %d", errno);
    this->mark_failed();
    return;
  }

    /* Start the serial connection back to the computer so that we can log
    messages to the Serial Monitor. Lets set the baud rate to 115200. */
  // Serial.begin(115200);

  /* Now we will install the DMX driver! We'll tell it which DMX port to use, 
    what device configure to use, and which interrupt priority it should have. 
    If you aren't sure which configuration or interrupt priority to use, you can
    use the macros `DMX_CONFIG_DEFAULT` and `DMX_INTR_FLAGS_DEFAULT` to set the
    configuration and interrupt to their default settings. */
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_driver_install(1, &config, DMX_INTR_FLAGS_DEFAULT);

  /* Now set the DMX hardware pins to the pins that we want to use and setup
    will be complete! */
  dmx_set_pin(1, TX_PIN, RX_PIN, EN_PIN);


  // join_igmp_groups_();
}

// 
// void E131Component::add_effect(E131LightEffectBase *light_effect) {
//   if (light_effects_.count(light_effect)) {
//     return;
//   }

//   ESP_LOGD(TAG, "Registering '%s' for universes %d-%d.", light_effect->get_name().c_str(),
//            light_effect->get_first_universe(), light_effect->get_last_universe());

//   light_effects_.insert(light_effect);

//   for (auto universe = light_effect->get_first_universe(); universe <= light_effect->get_last_universe(); ++universe) {
//     join_(universe);
//   }
// }

// void E131Component::remove_effect(E131LightEffectBase *light_effect) {
//   if (!light_effects_.count(light_effect)) {
//     return;
//   }

//   ESP_LOGD(TAG, "Unregistering '%s' for universes %d-%d.", light_effect->get_name().c_str(),
//            light_effect->get_first_universe(), light_effect->get_last_universe());

//   light_effects_.erase(light_effect);

//   for (auto universe = light_effect->get_first_universe(); universe <= light_effect->get_last_universe(); ++universe) {
//     leave_(universe);
//   }
// }

// void LALTDMXComponent::dmx_listen(){
//   const dmx_port_t dmx_num = DMX_NUM_1;

//   // Set communication pins and install the driver
//   dmx_config_t config = DMX_CONFIG_DEFAULT;
//   dmx_driver_install(dmx_num, &config, DMX_INTR_FLAGS_DEFAULT);
//   dmx_set_pin(dmx_num, TX_PIN, RX_PIN, EN_PIN);
//   dmx_set_baud_rate(dmx_num, DMX_BAUD_RATE); // Set DMX baud rate.
//   dmx_set_break_len(dmx_num, 94);            // Set DMX break length.
//   dmx_set_mab_len(dmx_num, 26);              // Set DMX MAB length.

//   dmx_packet_t packet;
//   bool is_connected = false;

//   TickType_t last_update = xTaskGetTickCount();

//   ESP_LOGI(TAG, "waiting for dmx connection");

//   while (true)
//   {
//     // Block until a packet is received
//     if (dmx_receive(dmx_num, &packet, DMX_TIMEOUT_TICK))
//     {
//       const TickType_t now = xTaskGetTickCount();

//       if (!is_connected)
//       {
//         // Log when we first connect
//         ESP_LOGI(TAG, "DMX is connected.");
//         is_connected = true;
//       }

//       {
//         dmx_read(dmx_num, data, ETC_PACKET_SIZE);
//         // ESP_LOGI(TAG, "error: %i", packet.err);
//         // ESP_LOGI(TAG, "Start code: %02x, Size: %i", packet.sc, packet.size);
//         // ESP_LOG_BUFFER_HEX(TAG, data, 16); // Log first 16 bytes

//         if (packet.err == 0 && packet.sc == 0x00 && packet.size == ETC_PACKET_SIZE)
//         {
//           // ESP_LOGI(TAG, "Start code: %02x, Size: %i", packet.sc, packet.size);
//           // ESP_LOG_BUFFER_HEX(TAG, data, ETC_PACKET_SIZE); // Log first 16 bytes

//           bool changed = false;
//           if (rgbw[0] != data[1] ||
//               rgbw[1] != data[2] ||
//               rgbw[2] != data[3] ||
//               rgbw[3] != data[4])
//           {
//             changed = true;
//             rgbw[0] = data[1];
//             rgbw[1] = data[2];
//             rgbw[2] = data[3];
//             rgbw[3] = data[4];
//           }

//           // if dmx data changed, send a udp packet
//           // if it's been 500ms since the last udp packet, send another one
//           if (changed || (now - last_update >= pdMS_TO_TICKS(500)))
//           {
//             udp_task();
//             last_update = now;
//           }
//         }
//       }
//     }
//     // else if (is_connected)
//     // {
//     //   // DMX timed out after having been previously connected
//     //   ESP_LOGI(TAG, "DMX was disconnected.");
//     // }
//   }

//   ESP_LOGI(TAG, "Uninstalling the DMX driver.");
//   dmx_driver_delete(dmx_num);
// }

// bool E131Component::process_(int universe, const E131Packet &packet) {
//   bool handled = false;

//   ESP_LOGV(TAG, "Received E1.31 packet for %d universe, with %d bytes", universe, packet.count);

//   for (auto *light_effect : this->light_effects_) {
//     // bool blah = light_effect->process_(universe, packet);
//     // ESP_LOGV(TAG, "blah: %d", universe, packet.count);
//     // if (!blah) {
//     //   return false;
//     // }
//     handled = light_effect->process_(universe, packet) || handled;
//   }

//   // for (auto *light_effect : this->light_effects_) {
//   //   bool success = light_effect->process_(universe, packet);
//   //   if (!success) {
//   //     return false;
//   //   }
//   // }

//   return handled;
// }

}  // namespace lalt_dmx
}  // namespace esphome
