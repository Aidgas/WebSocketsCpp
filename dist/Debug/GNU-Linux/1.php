<?php

if( ! isset($argv[1]) )
{
	exit('error arguments');
}

$v = trim( base64_decode( $argv[1] ) );

echo '+'.$v.'__'.date('d.m.Y i:H:s');
