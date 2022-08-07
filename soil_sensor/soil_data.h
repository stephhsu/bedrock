struct soil_data {
  uint32_t unix_timestamp;
  int moisture_value;
};

union outputToBt {
  soil_data data;
  byte btLine[6];
};
