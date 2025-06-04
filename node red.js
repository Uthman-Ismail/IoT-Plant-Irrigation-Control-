//Code used in the functions of node red


//Temperature
var temperature = msg.payload.temperature;
return {payload:temperature}

//Humidity
var humidity = msg.payload.humidity;
return {payload:humidity};

//Soil Moisture
var soil_moisture = msg.payload.soil;
return {payload:soil_moisture};

//decypt
const xorKey = 0x5A;
let encrypted = msg.payload.toString();  // ensure it's a string
let decrypted = "";

// XOR decryption loop
for (let i = 0; i < encrypted.length; i++) {
    decrypted += String.fromCharCode(encrypted.charCodeAt(i) ^ xorKey);
}

try {
    msg.payload = JSON.parse(decrypted);  // Convert back to JSON
} catch (err) {
    node.error("Decryption failed: " + err.message);
    msg.payload = { error: "Failed to decrypt or parse data." };
}

return msg;

//Temp
msg.payload = [
    {
      measurement: "plant",
      fields: {
        temperature: msg.payload,
      },
      tags: {
        sensorID: "1",
        location: "external_area"
      }
    }
  ];
  return msg;

  //Hum
  msg.payload = [
    {
      measurement: "plant",
      fields: {
        humidity: msg.payload,
      },
      tags: {
        sensorID: "2",
        location: "external_area"
      }
    }
  ];
  return msg;

  //Most
  msg.payload = [
    {
      measurement: "plant",
      fields: {
        humidity: msg.payload,
      },
      tags: {
        sensorID: "2",
        location: "external_area"
      }
    }
  ];
  return msg;


  //Function 2
  // Ensure input is an array
let input = msg.payload;

if (!Array.isArray(input)) {
    node.error("Expected msg.payload to be an array", msg);
    return null;
}

let data = {
    temperature: [],
    humidity: [],
    soil_moisture: []
};
let labels = [];

// Safely sort by timestamp
input = input.filter(row => row._time && row._field && row._value !== undefined);
input.sort((a, b) => new Date(a._time).getTime() - new Date(b._time).getTime());

// Format time and organize data
input.forEach(row => {
    let date = new Date(row._time);
    let timeLabel = date.toTimeString().slice(0, 5); // "HH:MM"

    // Add label if new
    if (!labels.includes(timeLabel)) {
        labels.push(timeLabel);
    }

    if (row._field === "temperature") {
        data.temperature.push(row._value);
    } else if (row._field === "humidity") {
        data.humidity.push(row._value);
    } else if (row._field === "soil_moisture") {
        data.soil_moisture.push(row._value);
    }
});

// Send data to ui_chart
msg.payload = [
    {
        series: ["temperature", "humidity", "soil_moisture"],
        data: [
            data.temperature,
            data.humidity,
            data.soil_moisture
        ],
        labels: labels
    }
];

return msg;