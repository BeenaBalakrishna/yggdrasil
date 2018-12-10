% Initialize input/output channels 
in_channel = CisInterface('CisPlyInput', 'inputB');
out_channel = CisInterface('CisPlyOutput', 'outputB');

flag = true;

% Loop until there is no longer input or the queues are closed
while flag

  % Receive input from input channel
  % If there is an error, the flag will be False.
  [flag, ply] = in_channel.recv();
  if (~flag)
    disp('Model B: No more input.');
    break;
  end;

  % Print received message
  fprintf('Model B: (%d verts, %d faces)\n', ply.nvert, ply.nface);
  fprintf('  Vertices:\n');
  for i = 1:int64(ply.nvert)
    fprintf('   %f, %f, %f\n', ...
            ply{'vertices'}{i}{1}, ply{'vertices'}{i}{2}, ply{'vertices'}{i}{3});
  end;
  fprintf('  Faces:\n');
  for i = 1:int64(ply.nface)
    fprintf('   %d, %d, %d\n', ...
            ply{'faces'}{i}{1}, ply{'faces'}{i}{2}, ply{'faces'}{i}{3});
  end;

  % Send output to output channel
  % If there is an error, the flag will be False
  flag = out_channel.send(ply);
  if (~flag)
    error('Model B: Error sending output.');
    break;
  end;
  
end;
