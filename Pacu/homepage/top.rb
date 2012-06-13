#!/usr/bin/env ruby

data_read = ''
process_times = Hash.new(0.0)

# Clear screen
print 27.chr + '[2J'

# Open a pipe to fs_usage for file calls.
data = IO.popen('fs_usage -wf filesys')

while true

  # Keep responsive to fs_usage output by reading 10 times per second.
  10.times do
    sleep 0.1

    data_read << data.read_nonblock(1048576) rescue ''
    lines = data_read.split("\n",-1)
    data_read = lines.pop.to_s

    lines.each do |line|
      elapsed,process = line[138..148],line[152..-1]
      process_times[process] += elapsed.to_f
    end
  end
  
  # Jump up to the top of the screen and print the header.
  print 27.chr + '[f'
  puts "PROCESS".ljust(30) + ' ' + 'TIME'
  puts "------------------------------ ---------"

  # Print in order of which used the most time.
  process_times.sort { |a,b| b[1] <=> a[1] }[0..19].each do |name,time|
    puts name[0,30].ljust(30) + ' ' + sprintf('%0.6f',time).rjust(9,'0')
  end

  # Clear the times for the next loop.
  process_times.each_key { |k| process_times[k] = 0.0 }

end
