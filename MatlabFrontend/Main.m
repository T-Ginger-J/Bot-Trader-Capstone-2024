input_val = 42.0;
output_val = SharedMemMex(input_val);
disp(['Output Value from C++: ', num2str(output_val)]);