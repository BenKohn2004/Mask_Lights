void Favero_Parser() {
  // Parses the Favero Data
  if (Serial.available() > 0) {
    // if ((Serial.available() > 0) or (millis() - lastRunTime > 1000)) {
    // lastRunTime = millis();

    // Create a place to hold the incoming message
    static char message[MAX_MESSAGE_LENGTH];
    static char prev_message[MAX_MESSAGE_LENGTH];

    char inByte = Serial.read();

    // Message coming in (check if starting character)
    if (inByte == STARTING_BYTE) {
      // Resets message position
      message_pos = 0;
      // Stores the Byte in the message position
      message[message_pos] = inByte;
      //increments message position
      message_pos++;
    } else if (message_pos < (MAX_MESSAGE_LENGTH - 1)) {
      // Stores the Byte in the message position
      message[message_pos] = inByte;
      //increments message position
      message_pos++;
    } else if (message_pos == (MAX_MESSAGE_LENGTH - 1)) {

      // Prints the Message if different from previous
      // Excudes the internal use byte in position 7 or check sum.
      if (message[1] != prev_message[1] or message[2] != prev_message[2] or message[3] != prev_message[3] or message[4] != prev_message[4] or message[5] != prev_message[5] or message[6] != prev_message[6] or message[8] != prev_message[8]) {
        // Sets New Data to True
        new_data = true;

        Serial.print("The message in HEX is: ");
        for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {
          Serial.print(message[i], HEX);
          Serial.print(",");
        }
        Serial.println("");

        // Assigns values from message to myData
        myData.Green_Light = bitRead(message[5], 3);
        myData.Red_Light = bitRead(message[5], 2);
        myData.White_Green_Light = bitRead(message[5], 1);
        myData.White_Red_Light = bitRead(message[5], 0);
        myData.Yellow_Green_Light = bitRead(message[5], 4);
        myData.Yellow_Red_Light = bitRead(message[5], 5);

        myData.Yellow_Card_Green = bitRead(message[8], 2);
        myData.Yellow_Card_Red = bitRead(message[8], 3);
        myData.Red_Card_Green = bitRead(message[8], 0);
        myData.Red_Card_Red = bitRead(message[8], 1);

        myData.Priority_Right = bitRead(message[6], 2);
        myData.Priority_Left = bitRead(message[6], 3);

        // Stores the Score and Time
        myData.Right_Score = hex_string_to_int(message[1]);
        myData.Left_Score = hex_string_to_int(message[2]);
        myData.Seconds_Remaining = hex_string_to_int(message[3]);
        myData.Minutes_Remaining = hex_string_to_int(message[4]);

        // Sets Previous Message to Current Message
        for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {
          prev_message[i] = message[i];
        }

        // Resets Message Position
        message_pos = 0;

        // Clears the Serial Buffer if more than Max Buffer Bytes bytes in the buffer
        if (Serial.available() > MAX_SERIAL_BUFFER_BYTES) {
          Serial.println("...............Clearing the Serial Buffer...............");
          while (Serial.available() > 0) {
            char inByte = Serial.read();
          }
        }
        // Activates Relays for Red/Green/White_Red/White_Green Lights
        if (myData.Green_Light == 1) {
          digitalWrite(GreenLights, HIGH);
        } else {
          digitalWrite(GreenLights, LOW);
        }
        if (myData.Red_Light == 1) {
          digitalWrite(RedLights, HIGH);
        } else {
          digitalWrite(RedLights, LOW);
        }
        if (myData.White_Green_Light == 1) {
          digitalWrite(White_GreenLights, HIGH);
        } else {
          digitalWrite(White_GreenLights, LOW);
        }
        if (myData.White_Red_Light == 1) {
          digitalWrite(White_RedLights, HIGH);
        } else {
          digitalWrite(White_RedLights, LOW);
        }
      }
    } else {
      Serial.println("Unexpected Message Position, Reseting to zero.");
      message_pos = 0;
    }
  }
}