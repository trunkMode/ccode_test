#!/usr/bin/perl -w
use strict;
use warnings;

sub is_wanted_files($)
{
     $_= shift;

     if (/boardData_.*\.bin/) {
        return 1;
     } else {
        return 0;
     }
}
sub get_bmi_bd_id($)
{
    $_ = shift;
    if (/QCA9984/) {
        if (/_2G_/) {
            return 4;
        } elsif (/_5G_/) {
	    return 3;
        } else {
	    die "Do you have named you board data files in correct format?";
        }
    }
    if (/QCA4019/) {
          return 0;
    } else {
       die "Do you put board data files into firmware/QCA4019/hw1.0?";
    }
}

my ($DIR_PATH, $magic_hdr_file, $bmi_bd_id) = @ARGV;
my $bmi_bd_id_generate = 0;
my @file_list=();

#58
my $magic_bin_hdr;
my $magic_hdr = "5143412d41544831304b2d424f415244006d6d6d00000000542f000000000000240000006275733d7063692c626d692d636869702d69643d302c";
my $magic_hdr_end = "01000000202f0000"; #8
my $magic_default_len = 80;
my $max_len = 1024*1024;

my ($file, $bin, $ret) = ();
my $magic_key = "QCA-ATH10K-BOARD";
my $magic_key_len = length($magic_key);

#if (!defined($magic_hdr_file)) {
#    die "Please specific the files including correct magic header files\n";
#} else {
#    open MAGIC_HDR, "<$magic_hdr_file" or die "can't open $magic_hdr_file \n";
#    $ret = read MAGIC_HDR, $magic_hdr, $magic_default_len, 0;

#    if (!$ret || $ret != $magic_default_len) {
#         die "Failed to read magic $magic_hdr_file\n";
#    }
#}

###########test ####################
#open MAGIC_WW, ">magichdr.bin" or die "can't open xxx \n";

# 转ASCII
#$magic_hdr = pack('H*', $magic_hdr)."bmi-board-id".pack('H*', $magic_hdr_end);
#syswrite MAGIC_WW, $magic_hdr, 80;
#print "$magic_hdr \n";         
############################################

if (!defined($DIR_PATH)) {
    $DIR_PATH = ".";
}

if (!defined($bmi_bd_id)) {
    $bmi_bd_id_generate = 1;
}
opendir DIR_HANDLE, ${DIR_PATH} or die "Can not open $DIR_PATH \n";
@file_list = readdir DIR_HANDLE;

foreach (@file_list) {
   $file = $DIR_PATH."/$_";
   
   if (!is_wanted_files($file)) {
      next;
   }

   open BD_FD, "<$file" or die "Can't open $file \n";
   binmode BD_FD;
   $ret = read BD_FD, $bin, $magic_key_len, 0;

   if ($ret == $magic_key_len) {
       if ($bin eq $magic_key) {
	    close BD_FD;
       } else {
	    printf("$DIR_PATH/$_ has no magic header, write one\n"); 

	    #generate magic hdr
	    if ($bmi_bd_id_generate) {
	        $bmi_bd_id = get_bmi_bd_id($file);
		    printf("bmi_bd_id === %d\n", $bmi_bd_id);
	    }
	    $magic_bin_hdr = pack('H*', $magic_hdr)."bmi-board-id="."$bmi_bd_id".pack('H*', $magic_hdr_end);
	    open NEW_BD_FD, ">$file.tmp" or die "can't open $file.tmp to write\n";
            binmode(NEW_BD_FD);
            syswrite NEW_BD_FD, $magic_bin_hdr, $magic_default_len, 0;
	    
	    seek BD_FD, 0, 0;
	    $ret = read BD_FD, $bin, $max_len, 0;
	    syswrite NEW_BD_FD, $bin, $ret;
            close BD_FD;
  
	   # system("mv $file.tmp $file");
	      
       }
   } else {
        close BD_FD;
        printf("can't read magic_key from file $DIR_PATH/$_ \n");
   }
   
}

#open (FF, '<000001.dat') or die "no such file\n";
#binmode (FF);
#read (FF, $bindata, 8);
#close (FF);
#my ($id, $name) = unpack ('la*', $bindata);
#print $id, $name; 
