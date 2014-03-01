typedef struct MTR_CTRL_TAG {
  const uint16_t address;               // motor address (readonly)
  uint16_t       steps_left;            // steps remaining
  uint16_t       dir;                   // direction (CW/CCW)
  uint16_t       step_index;            // stepper waveform index
  int16_t        offset;                // offset counter
} MTR_CTRL;

typedef struct MTR_CHANNELS_TAG {
  int16_t        channel_1;             // gripper
  int16_t        channel_2;             // left wrist action
  int16_t        channel_3;             // right wrist action
  int16_t        channel_4;             // elbow
  int16_t        channel_5;             // shoulder
  int16_t        channel_6;             // base
} MTR_CHANNELS;

