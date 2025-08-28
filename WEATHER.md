# Weather Feature for OLSH 🌤️

A cool new weather command written in Ruby that provides beautiful, colorful weather information right in your terminal!

## Features

- 🌡️ **Current temperature** with "feels like" temperature
- ☁️ **Weather conditions** with descriptive text
- 💧 **Humidity and atmospheric data** (pressure, visibility, wind)
- 📅 **Daily forecast** with high/low temperatures  
- ⏰ **Hourly forecast** for the current day
- 🌈 **Colorful, emoji-rich output** that's easy to read
- 💡 **Smart weather tips** based on current conditions
- 🌍 **Support for any city worldwide**

## Usage

```bash
# Get weather for current/default location
weather

# Get weather for a specific city
weather Tokyo
weather "New York"
weather London
weather "San Francisco"
```

## Requirements

- **Ruby** must be installed on your system
- **Internet connection** for fetching weather data
- The weather data is fetched from wttr.in (no API key required!)

## Examples

### Basic Usage
```bash
$ weather Tokyo
🌤️  Fetching weather for Tokyo...

📍 Weather in Tokyo
──────────────────────────────────────────────────
🌡️  Temperature: 23°C / 73°F
🤔 Feels like: 25°C / 77°F
☁️  Conditions: Partly cloudy
💧 Humidity: 65%
💨 Wind: 12 km/h NE
🔽 Pressure: 1013 hPa
👁️  Visibility: 10 km

📅 Today's Forecast
──────────────────────────────────────────────────
🌡️  High/Low: 28°C / 18°C

⏰ Hourly Forecast
──────────────────────────────────────────────────
00:00 - 20°C, Clear ☀️ 5%
03:00 - 18°C, Clear ☀️ 0%
06:00 - 22°C, Partly cloudy ⛅ 15%
09:00 - 26°C, Sunny ☀️ 10%
12:00 - 28°C, Sunny ☀️ 5%
15:00 - 27°C, Partly cloudy ⛅ 20%
18:00 - 24°C, Partly cloudy ⛅ 25%
21:00 - 21°C, Clear ☀️ 10%

✨ Fun Weather Facts
──────────────────────────────────────────────────
☀️ Perfect weather for outdoor activities!
😎 Great day for sunglasses!
```

## Installation Notes

If Ruby is not installed, the command will show helpful installation instructions:

```
❌ Error: Ruby is not installed or not in PATH
💡 Please install Ruby to use the weather command
   Visit: https://www.ruby-lang.org/en/downloads/
```

## Configuration

You can set a default location by setting the `WEATHER_LOCATION` environment variable:

```bash
# In PowerShell
$env:WEATHER_LOCATION = "Tokyo"

# In bash/zsh
export WEATHER_LOCATION="Tokyo"
```

## Demo Script

Try running the weather demo script:
```bash
./docs/examples/weather-demo.olsh
```

This will show weather for multiple cities around the world!

---

*Weather data provided by wttr.in - a console-oriented weather forecast service*
