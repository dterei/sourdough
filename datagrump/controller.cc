#include <algorithm>
#include <iostream>
#include <queue>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

const int64_t DEFAULT_WINDOW = 5;
const int64_t SAMPLES_SHORT = 10;
const int64_t SAMPLES_LONG = 10 * SAMPLES_SHORT;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_{ debug }, win_{ DEFAULT_WINDOW }, queue_{0},
    avg_s_{}, avg_r_{}, avg_t_{},
    avg_s_l_{}, avg_r_l_{}, avg_t_l_{},
    rtt_avg_short_{0}, rtt_avg_long_{0}, rtt_sq_long_{0}
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << win_ << endl;
  }

  return win_;
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
    cerr << "Pop: " << last_short << ", ";
    avg_s_.pop();
    avg_r_.pop();
    avg_t_.pop();
    // simple moving average
    rtt_avg_short_ += (new_sample - last_short) / SAMPLES_SHORT;
  } else {
    rtt_avg_short_ = (rtt_avg_short_ * (avg_s_.size() - 1) + new_sample) / avg_s_.size();
  }

  avg_s_l_.push(recv_timestamp_acked - send_timestamp_acked);
  avg_r_l_.push(timestamp_ack_received - recv_timestamp_acked);
  avg_t_l_.push(new_sample);
  bool cont = true;

  if ( avg_s_l_.size() > SAMPLES_LONG ) {
    int64_t last_long = avg_t_l_.front();
    cerr << "Last: " << last_long << ", " << endl;
    avg_s_l_.pop();
    avg_r_l_.pop();
    avg_t_l_.pop();
    // simple moving average
    rtt_avg_long_ += (new_sample - last_long) / SAMPLES_LONG;
    rtt_sq_long_ += (new_sample * new_sample - last_long * last_long);
  } else {
    rtt_avg_long_ = (rtt_avg_long_ * (avg_s_l_.size() - 1) + new_sample) / avg_s_l_.size();
    rtt_sq_long_ += new_sample * new_sample;
    cont = false;
  }

  int64_t rtt_dev = abs(rtt_sq_long_ - rtt_avg_long_ * rtt_avg_long_ * SAMPLES_LONG);
  int64_t rtt_dev_root = sqrt(rtt_dev / SAMPLES_LONG);

  cerr << "Short: " << rtt_avg_short_
       << ", Long: " << rtt_avg_long_
       << ", Sq: " << rtt_sq_long_
       << ", Dev: " << rtt_dev
       << ", Dev': " << rtt_dev_root
       << ", Win: " << win_;

  if ( !cont ) {
    cerr << endl;
    return;
  }

  if ( rtt_avg_short_ > rtt_avg_long_ + rtt_dev ) {
    /* reduce window */
    win_--;
    cerr << ", --" << endl;
  } else if ( rtt_avg_short_ < rtt_avg_long_ - rtt_dev ) {
    /* increase window */
    win_++;
    cerr << ", ++" << endl;
  } else {
    /* leave window unchanged */
    cerr << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
