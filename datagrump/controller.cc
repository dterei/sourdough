#include <algorithm>
#include <iostream>
#include <queue>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

const int SCALE = 10;
const int UP = 2;
const int DOWN = SCALE;
const int64_t DEFAULT_WINDOW = 30 * SCALE;
const int64_t SAMPLES_SHORT = 4;
const int64_t SAMPLES_LONG = 20;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_{ debug }, win_{ DEFAULT_WINDOW }, queue_{0},
    avg_s_{}, avg_r_{}, avg_t_{},
    avg_s_l_{}, avg_r_l_{}, avg_t_l_{},
    rtt_sum_short_{0}, rtt_sum_long_{0}, rtt_sq_long_{0}
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << win_ << endl;
  }

  return win_ / SCALE;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
                                    /* of the sent datagram */
                                    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
         << " sent datagram " << sequence_number << endl;
  }

  queue_++;
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
                               /* what sequence number was acknowledged */
                               const uint64_t send_timestamp_acked,
                               /* when the acknowledged datagram was sent (sender's clock) */
                               const uint64_t recv_timestamp_acked,
                               /* when the acknowledged datagram was received (receiver's clock)*/
                               const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (send @ time " << send_timestamp_acked
         << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
         << endl;
  }

  queue_--;

  int64_t new_sample = timestamp_ack_received - send_timestamp_acked;

  cerr << "Sample: " << new_sample << endl;

  avg_s_.push(recv_timestamp_acked - send_timestamp_acked);
  avg_r_.push(timestamp_ack_received - recv_timestamp_acked);
  avg_t_.push(new_sample);

  if ( avg_s_.size() > SAMPLES_SHORT ) {
    int64_t last_short = avg_t_.front();
    avg_s_.pop();
    avg_r_.pop();
    avg_t_.pop();
    rtt_sum_short_ += new_sample - last_short;
  } else {
    rtt_sum_short_ += new_sample;
  }

  avg_s_l_.push(recv_timestamp_acked - send_timestamp_acked);
  avg_r_l_.push(timestamp_ack_received - recv_timestamp_acked);
  avg_t_l_.push(new_sample);

  bool cont = true;

  if ( avg_s_l_.size() > SAMPLES_LONG ) {
    int64_t last_long = avg_t_l_.front();
    avg_s_l_.pop();
    avg_r_l_.pop();
    avg_t_l_.pop();
    rtt_sum_long_ += new_sample - last_long;
    rtt_sq_long_  += new_sample * new_sample - last_long * last_long;
  } else {
    rtt_sum_long_ += new_sample;
    rtt_sq_long_  += new_sample * new_sample;
    cont = false;
  }

  int64_t rtt_dev = (rtt_sq_long_ / SAMPLES_LONG)
    - (rtt_sum_long_ * rtt_sum_long_) / (SAMPLES_LONG * SAMPLES_LONG);
  int64_t rtt_dev_root = sqrt(abs(rtt_dev));

  int64_t rtt_avg_short = rtt_sum_short_ / SAMPLES_SHORT;
  int64_t rtt_avg_long  = rtt_sum_long_  / SAMPLES_LONG;

  cerr << "Short: " << rtt_avg_short
       << ", Long: " << rtt_avg_long
       << ", Sq: " << rtt_sq_long_
       << ", Dev: " << rtt_dev
       << ", Dev': " << rtt_dev_root
       << ", Win: " << win_ / SCALE;

  if ( !cont ) {
    cerr << endl;
    return;
  }

  if ( rtt_avg_short > rtt_avg_long + rtt_dev_root ) {
    /* reduce window */
    win_ -= DOWN;
    if (win_ <= SCALE) { win_ = SCALE; }
    cerr << ", --" << endl;
  } else if ( rtt_avg_short < rtt_avg_long - rtt_dev_root ) {
    /* increase window */
    win_ += UP;
    cerr << ", ++" << endl;
  } else {
    /* leave window unchanged */
    win_ += UP;
    cerr << ", +*" << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
