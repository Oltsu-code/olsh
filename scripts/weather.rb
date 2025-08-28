#!/usr/bin/env ruby
# Weather fetcher for OLSH
# Usage: ruby weather.rb [city]

require 'net/http'
require 'json'
require 'uri'

class WeatherFetcher
  API_BASE = "http://wttr.in"
  
  def initialize
    @colors = {
      reset: "\e[0m",
      bold: "\e[1m",
      cyan: "\e[36m",
      yellow: "\e[33m",
      green: "\e[32m",
      blue: "\e[34m",
      red: "\e[31m",
      magenta: "\e[35m"
    }
  end
  
  def fetch_weather(city = nil)
    location = city || get_current_location
    
    if location.nil? || location.empty?
      puts "#{@colors[:red]}Error: Please specify a city#{@colors[:reset]}"
      puts "Usage: ruby weather.rb [city]"
      return
    end
    
    puts "#{@colors[:cyan]}🌤️  Fetching weather for #{@colors[:bold]}#{location}#{@colors[:reset]}#{@colors[:cyan]}...#{@colors[:reset]}\n\n"
    
    # Fetch weather data from wttr.in
    begin
      uri = URI("#{API_BASE}/#{URI.encode_www_form_component(location)}?format=j1")
      response = Net::HTTP.get_response(uri)
      
      if response.code == '200'
        weather_data = JSON.parse(response.body)
        display_weather(weather_data, location)
      else
        puts "#{@colors[:red]}Error: Could not fetch weather data#{@colors[:reset]}"
      end
    rescue => e
      puts "#{@colors[:red]}Error: #{e.message}#{@colors[:reset]}"
      puts "\n#{@colors[:yellow]}💡 Tip: Make sure you have an internet connection#{@colors[:reset]}"
    end
  end
  
  private
  
  def get_current_location
    # Try to get location from environment or use a default
    ENV['WEATHER_LOCATION'] || 'London'
  end
  
  def display_weather(data, location)
    current = data['current_condition'][0]
    today = data['weather'][0]
    
    # Current conditions
    puts "#{@colors[:bold]}#{@colors[:blue]}📍 Weather in #{location}#{@colors[:reset]}"
    puts "#{@colors[:cyan]}─" * 50 + "#{@colors[:reset]}"
    
    # Temperature and description
    temp_c = current['temp_C']
    temp_f = current['temp_F']
    feels_like_c = current['FeelsLikeC']
    feels_like_f = current['FeelsLikeF']
    description = current['weatherDesc'][0]['value']
    
    puts "#{@colors[:yellow]}🌡️  Temperature:#{@colors[:reset]} #{temp_c}°C / #{temp_f}°F"
    puts "#{@colors[:yellow]}🤔 Feels like:#{@colors[:reset]} #{feels_like_c}°C / #{feels_like_f}°F"
    puts "#{@colors[:green]}☁️  Conditions:#{@colors[:reset]} #{description}"
    
    # Additional info
    humidity = current['humidity']
    wind_speed = current['windspeedKmph']
    wind_dir = current['winddir16Point']
    pressure = current['pressure']
    visibility = current['visibility']
    
    puts "#{@colors[:blue]}💧 Humidity:#{@colors[:reset]} #{humidity}%"
    puts "#{@colors[:blue]}💨 Wind:#{@colors[:reset]} #{wind_speed} km/h #{wind_dir}"
    puts "#{@colors[:blue]}🔽 Pressure:#{@colors[:reset]} #{pressure} hPa"
    puts "#{@colors[:blue]}👁️  Visibility:#{@colors[:reset]} #{visibility} km"
    
    # Today's forecast
    puts "\n#{@colors[:bold]}#{@colors[:magenta]}📅 Today's Forecast#{@colors[:reset]}"
    puts "#{@colors[:cyan]}─" * 50 + "#{@colors[:reset]}"
    
    max_temp = today['maxtempC']
    min_temp = today['mintempC']
    
    puts "#{@colors[:yellow]}🌡️  High/Low:#{@colors[:reset]} #{max_temp}°C / #{min_temp}°C"
    
    # Hourly forecast for today
    puts "\n#{@colors[:bold]}⏰ Hourly Forecast#{@colors[:reset]}"
    puts "#{@colors[:cyan]}─" * 50 + "#{@colors[:reset]}"
    
    today['hourly'].each do |hour|
      time = hour['time'].to_i / 100
      temp = hour['tempC']
      desc = hour['weatherDesc'][0]['value']
      chance_rain = hour['chanceofrain']
      
      time_str = sprintf("%02d:00", time)
      puts "#{@colors[:cyan]}#{time_str}#{@colors[:reset]} - #{temp}°C, #{desc} #{get_rain_emoji(chance_rain.to_i)} #{chance_rain}%"
    end
    
    # Fun facts
    puts "\n#{@colors[:bold]}#{@colors[:yellow]}✨ Fun Weather Facts#{@colors[:reset]}"
    puts "#{@colors[:cyan]}─" * 50 + "#{@colors[:reset]}"
    puts get_weather_tip(temp_c.to_i, description.downcase, humidity.to_i)
  end
  
  def get_rain_emoji(chance)
    case chance
    when 0...20
      "☀️"
    when 20...40
      "⛅"
    when 40...60
      "🌥️"
    when 60...80
      "🌦️"
    else
      "🌧️"
    end
  end
  
  def get_weather_tip(temp, description, humidity)
    tips = []
    
    if temp < 0
      tips << "🧊 It's freezing! Bundle up and watch for ice!"
    elsif temp < 10
      tips << "🧥 Pretty chilly - grab a coat!"
    elsif temp > 30
      tips << "🔥 It's hot! Stay hydrated and find some shade!"
    elsif temp > 25
      tips << "☀️ Perfect weather for outdoor activities!"
    end
    
    if description.include?('rain') || description.include?('shower')
      tips << "☔ Don't forget your umbrella!"
    elsif description.include?('snow')
      tips << "❄️ Snow day! Perfect for hot cocoa!"
    elsif description.include?('sun') || description.include?('clear')
      tips << "😎 Great day for sunglasses!"
    end
    
    if humidity > 80
      tips << "💦 It's quite humid - you might feel sticky!"
    elsif humidity < 30
      tips << "🏜️ Low humidity - your skin might feel dry!"
    end
    
    tips.empty? ? "🌤️ Have a great day!" : tips.join("\n")
  end
end

# Main execution
if __FILE__ == $0
  city = ARGV[0]
  weather = WeatherFetcher.new
  weather.fetch_weather(city)
end
