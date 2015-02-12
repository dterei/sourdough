#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
#include <queue>

/* Congestion controller interface */

class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */
  int win_;

  unsigned int queue_;

  /* previous N RTT samples */
  std::queue<uint64_t> avg_s_;
  std::queue<uint64_t> avg_r_;
  std::queue<uint64_t> avg_t_;

  /* previous >>N RTT samples */
  std::queue<uint64_t> avg_s_l_;
  std::queue<uint64_t> avg_r_l_;
  std::queue<uint64_t> avg_t_l_;

  /* RTT averages */
  int64_t rtt_sum_short_;
  int64_t rtt_sum_long_;
  int64_t rtt_sq_long_;

public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  unsigned int window_size( void );

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
                          const uint64_t send_timestamp );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
                     const uint64_t send_timestamp_acked,
                     const uint64_t recv_timestamp_acked,
                     const uint64_t timestamp_ack_received );

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );
};

#endif
