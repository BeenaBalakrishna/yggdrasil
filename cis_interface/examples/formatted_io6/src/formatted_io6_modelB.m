% Initialize input/output channels 
in_channel = CisInterface('CisObjInput', 'inputB');
out_channel = CisInterface('CisObjOutput', 'outputB');

flag = true;

% Loop until there is no longer input or the queues are closed
while flag

  % Receive input from input channel
  % If there is an error, the flag will be False.
  [flag, obj] = in_channel.recv();
  if (~flag)
    disp('Model B: No more input.');
    break;
  end;

  % Print received message
  fprintf('Model B: (%d verts, %d faces)\n', obj.nvert, obj.nface);
  fprintf('  Vertices:\n');
  for i = 1:int64(obj.nvert)
    fprintf('   %f, %f, %f\n', ...
	    obj{'vertices'}{i}{1}, obj{'vertices'}{i}{2}, obj{'vertices'}{i}{3});
  end;
  fprintf('  Faces:\n');
  for i = 1:int64(obj.nface)
    fprintf('   %d, %d, %d\n', ...
	    obj{'faces'}{i}{1}, obj{'faces'}{i}{2}, obj{'faces'}{i}{3});
  end;

  % Send output to output channel
  % If there is an error, the flag will be False
  flag = out_channel.send(obj);
  if (~flag)
    error('Model B: Error sending output.');
    break;
  end;
  
end;
