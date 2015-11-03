/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple�s copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.  */
 
/*******************************************************************************
	File:		USBPrinterBrowser.r
	
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
				
*******************************************************************************/

#include <Carbon/Carbon.r>

resource 'STR#' (1000, "Column Titles") {
	{
		" ",
		"Product",
                "Kind"
	}
};

data 'icl4' (1001, purgeable) {
	$"000F FFFF FFFF FFFF FFFF FF00 0000 0000 000F C0C0 C0C0 C0C0 C0C0 CFF0 0000 0000"                    /* ..���������.......����������.... */
	$"000F 0C0C 0555 0C0C 0C0C 0FCF 0000 0000 000F C0C0 55C0 50C0 C0C0 CFCC F000 0000"                    /* .....U.....�......��U�P����Õ... */
	$"000F 0C0C 5507 5C0C 0C0C 0FCC CF00 0000 000F C0C0 5575 70C0 C0C0 CFCC CCF0 0000"                    /* ....U.\....æ.....��Uup�����Õ.. */
	$"000F 0C07 5557 0C0C 0C0C 0FFF FFFF 0000 000F C075 5570 C555 50C0 C0C0 C0CF 0000"                    /* ....UW.....���....�uUp�UP�����.. */
	$"000F 0C55 055C 0C55 0CDD DC0C 0C0F 0000 000F C550 C757 C750 CDD0 CDC0 C0CF 0000"                    /* ...U.\.U.��.......�P�W�Pխտ��.. */
	$"000F 055C 0C55 750C 0DDC CD0C 0C0F 0000 000F C550 C075 50C0 CDDC DCC0 C0CF 0000"                    /* ...\.Uu..��.......�P�uP���п��.. */
	$"000F 0555 7755 5555 CDDD CC0C 0C0F 0000 000F C055 5570 C55C DDDC C0DD DDCF 0000"                    /* ...UwUUU���.......�UUp�\�п��.. */
	$"0FFF 0C0C 0C0C 0C0D DCDD 0C0D DC0F FF00 FDDF C0C0 C0C0 C0DD C0CD C0CD C0CF EDF0"                    /* .�......��..�.�.��������տտ�̕ */
	$"F00F 0C0C 0C0C 0CDD 0C0D DCDC 0C0F E0F0 FCCF C0C0 C0C0 C0DD C0CC DDC0 CDCF ECF0"                    /* �......�..��..������������զϕ */
	$"FDDF 0C0C 0C0C 0CDD DCCD DDDD DD0F EDF0 FEEF C0C0 C0C0 C0CD DDDC C0DD D0CF EEF0"                    /* ��.....������.̕�Կ������п�ӕ */
	$"0FFF 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F FF00 0FFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"                    /* .�............�..��������������. */
	$"FDDD DDDD DDDD DDDD DDDD DDDD DDDD DDF0 F000 0000 0000 0000 0000 0000 0000 00F0"                    /* ���������������..............� */
	$"FCCC CCCC CCCC CCCC CCCC CCCC CCCC CCF0 FDDD DDDD DDDD DDDD DDDD DDDD DDDD DDF0"                    /* ��������������Õ��������������� */
	$"FEEE EEEE EEEE EEEE EEEE EEEE EEEE EEF0 0FFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"                    /* ��������������ӕ.��������������. */
	$"000F EEEE EEEE EEEE EEEE EEEE EEEF 0000 000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"                    /* ..������������.................. */
	$"000F C0C0 C0C0 C0C0 C0C0 C0C0 C0CF 0000 000F FFFF FFFF FFFF FFFF FFFF FFFF 0000"                    /* ..������������....������������.. */
};

data 'icl4' (1000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 DDDD DDDD DDDD DDDD DDDD DDDD DDDD DDD0"                    /* ................��������������� */
	$"DCCC CCCC CCCC CCCC CCCC CCCC ECCC CCD0 DCC2 CCCC CCCC CCCC CCCC CCCC EDDD DDDD"                    /* ��������������íЬ�������������� */
	$"DCCC CCCC CCCC CCCC CCCC CCCC ECCC CCCD DCCC CCCC CCCC CCCC CCCC CCCC ECCC CECD"                    /* �����������������������������Ñ� */
	$"DCCC CCCC CCCC CCCC CCCC CCCC ECCC CCCD DCCC CCCC CCCC CCCC CCCC CCCC ECCC CCDD"                    /* �������������������������������� */
	$"DCCC CCCC CCCC CCCC CCCC CCCC ECCC C8CD DCCC CCCC CCCC CCCC CCCC CCCC ECCC CCDD"                    /* �������������û����������������� */
	$"DCCC CCCC CCCC CCCC CCCC CCCC ECCC CDCD DCCC CCCC CCCC CCCC CCCC CCCC ECCC CCCD"                    /* �������������������������������� */
	$"DDDD DEEE EEEE EEEE EEEE EEEE ECCC CCCD DCCD DDCD EEEE EEEE EEDD EEDC DDDE EECD"                    /* �������������������������������� */
	$"DCCD CDCD CE66 8833 2211 ECDC DCDC DDCD DCCD CDCD EE66 8833 2211 EEDC DCDE EECD"                    /* ���Ցf�3".�����������f�3".������ */
	$"DCCD CDCD 0066 8833 2211 00DC DCDC DDCD DCCD CDCD 0066 8833 2211 00DC DCDE EECD"                    /* ����.f�3"..���������.f�3"..����� */
	$"DCCD CDCD 0000 0000 0000 00DC DCDC CCCD DDDD CDCE EEEE EEEE EEEE EEDC DCDD DDDD"                    /* ����.......�������Ց������������ */
	$"0EED CDCE ECCC CCCC CCCE EEEC DCDE EEE0 0DDD CDDD DDDD DDDD DDDD DDDD DCDD DDD0"                    /* .�Ց����Ñ������.�������������� */
	$"000D DEEE EEEE EEEE EEEE EEEE EDD0 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ..����������̭.................. */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
};

data 'icl8' (1001, purgeable) {
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000 0000 0000 0000"                    /* ...�������������������.......... */
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5FF FF00 0000 0000 0000 0000"                    /* ...��������������������......... */
	$"0000 00FF F5F5 F5F5 F5B0 B0B0 F5F5 F5F5 F5F5 F5F5 F5FF 2BFF 0000 0000 0000 0000"                    /* ...�������������������+�........ */
	$"0000 00FF F5F5 F5F5 B0B0 F5F5 B0F5 F5F5 F5F5 F5F5 F5FF 2B2B FF00 0000 0000 0000"                    /* ...�������������������++�....... */
	$"0000 00FF F5F5 F5F5 B0B0 F554 B0F5 F5F5 F5F5 F5F5 F5FF 2B2B 2BFF 0000 0000 0000"                    /* ...��������T����������+++�...... */
	$"0000 00FF F5F5 F5F5 B0B0 54B0 54F5 F5F5 F5F5 F5F5 F5FF 2B2B 2B2B FF00 0000 0000"                    /* ...�������T�T���������++++�..... */
	$"0000 00FF F5F5 F554 B0B0 B054 F5F5 F5F5 F5F5 F5F5 F5FF FFFF FFFF FFFF 0000 0000"                    /* ...����T���T����������������.... */
	$"0000 00FF F5F5 54B0 B0B0 54F5 F5B0 B0B0 B0F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"                    /* ...���T���T�����������������.... */
	$"0000 00FF F5F5 B0B0 F5B0 B0F5 F5F5 B0B0 F5F5 F9F9 F9F5 F5F5 F5F5 F5FF 0000 0000"                    /* ...�������������������������.... */
	$"0000 00FF F5B0 B0F5 F554 B054 F554 B0F5 F5F9 F9F5 F5F9 F5F5 F5F5 F5FF 0000 0000"                    /* ...������T�T�T��������������.... */
	$"0000 00FF F5B0 B0F5 F5F5 B0B0 54B0 F5F5 F5F9 F9F5 F7F9 F5F5 F5F5 F5FF 0000 0000"                    /* ...���������T���������������.... */
	$"0000 00FF F5B0 B0F5 F5F5 54B0 B0F5 F5F5 F5F9 F9F7 F9F7 F5F5 F5F5 F5FF 0000 0000"                    /* ...�������T�����������������.... */
	$"0000 00FF F5B0 B0B0 5454 B0B0 B0B0 B0B0 F7F9 F9F9 F7F5 F5F5 F5F5 F5FF 0000 0000"                    /* ...�����TT������������������.... */
	$"0000 00FF F5F5 B0B0 B0B0 54F5 F5B0 B0F7 F9F9 F9F7 F5F5 F9F9 F9F9 F5FF 0000 0000"                    /* ...�������T�����������������.... */
	$"00FF FFFF F5F5 F5F5 F5F5 F5F5 F5F5 F5F9 F9F5 F9F9 F5F5 F5F9 F9F5 F5FF FFFF 0000"                    /* .�����������������������������.. */
	$"FFF9 F9FF F5F5 F5F5 F5F5 F5F5 F5F5 F9F9 F5F5 F7F9 F7F5 F7F9 F5F5 F5FF F9F9 FF00"                    /* �������������������������������. */
	$"FFF5 F5FF F5F5 F5F5 F5F5 F5F5 F5F5 F9F9 F5F5 F5F9 F9F7 F9F5 F5F5 F5FF F9F5 FF00"                    /* �������������������������������. */
	$"FFF7 F7FF F5F5 F5F5 F5F5 F5F5 F5F5 F9F9 F5F5 F5F7 F9F9 F5F5 F5F9 F5FF F9F7 FF00"                    /* �������������������������������. */
	$"FFF9 F9FF F5F5 F5F5 F5F5 F5F5 F5F5 F9F9 F9F7 F7F9 F9F9 F9F9 F9F9 F5FF F9F9 FF00"                    /* �������������������������������. */
	$"FFFC FCFF F5F5 F5F5 F5F5 F5F5 F5F5 F5F9 F9F9 F9F7 F5F5 F9F9 F9F5 F5FF FCFC FF00"                    /* �������������������������������. */
	$"00FF FFFF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5FF FFFF 0000"                    /* .�����������������������������.. */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"                    /* .�����������������������������.. */
	$"FFF9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 FF00"                    /* �������������������������������. */
	$"FFF5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 FF00"                    /* �������������������������������. */
	$"FFF7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 F7F7 FF00"                    /* �������������������������������. */
	$"FFF9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 F9F9 FF00"                    /* �������������������������������. */
	$"FFFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FCFC FF00"                    /* �������������������������������. */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"                    /* .�����������������������������.. */
	$"0000 00FF FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFF 0000 0000"                    /* ...�������������������������.... */
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"                    /* ...�������������������������.... */
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"                    /* ...�������������������������.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"                    /* ...�������������������������.... */
};

data 'icl8' (1000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FA00"                    /* �������������������������������. */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 F6F6 FA00"                    /* �+�����������������������������. */
	$"FA2B 2B16 F5F5 F7F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF8 F8F8 F8F8 F9FA"                    /* �++.���������������������������� */
	$"FA2B F7F8 F8F8 2BF6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 F6F6 2BFA"                    /* �+����+�����������������������+� */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 F6FB 2BFA"                    /* �+����������������������������+� */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 2BF8 2BFA"                    /* �+��������������������������+�+� */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 F8F6 F9FA"                    /* �+������������������������������ */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 2BE3 F7FA"                    /* �+��������������������������+��� */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 F8F6 F9FA"                    /* �+������������������������������ */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 2BF9 F7FA"                    /* �+��������������������������+��� */
	$"FA2B F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F6F6 F6F6 2BFA"                    /* �+����������������������������+� */
	$"FAF9 F9F9 F9FB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBF6 F6F6 F6F6 2BFA"                    /* ������������������������������+� */
	$"FA2B F6FA FAFA F6FA FDFD FCFC FBFB FCFC FBFB FAFA FDFD FAF6 FAFA FAFB FCFC 2BFA"                    /* �+����������������������������+� */
	$"FA2B F6FA F6FA F6FA F8FB EBEC BFE3 D8D7 1717 0505 FBF7 FAF6 FAF7 FAF7 F9F9 2BFA"                    /* �+��������������....����������+� */
	$"FA2B F6FA F6F9 F6FA FBFB EBEC BFE3 D8D7 1717 0505 FBFB FAF6 F9F7 FAFB FCFC 2BFA"                    /* �+��������������....����������+� */
	$"FA2B F6FA F6F9 F6FA 0000 ECEB E3E3 D7D7 1717 0505 0000 FAF6 F9F7 FAF7 F9F9 2BFA"                    /* �+������..�Γ���......��������+� */
	$"FA2B F6FA F6F9 F6FA 0000 EBEC E3BF D8D7 1717 0505 0000 FAF6 F9F7 FAFB FCFC 2BFA"                    /* �+������..�ϓ���......��������+� */
	$"FA2B F6FA F6F9 F6FA 0000 0000 0000 0000 0000 0000 0000 FAF6 F9F7 FAF6 F6F6 2BFA"                    /* �+������..............��������+� */
	$"FAFA FAFA F6F9 F6FD FBFB FBFB FBFB FBFB FBFB FBFB FBFB FAF6 F9F7 FAFA FAFA FAFA"                    /* �������������������������������� */
	$"00FB FCFA F6F9 F6FD FCF7 F7F7 F7F7 F7F7 F7F7 F7FC FCFC FCF6 F9F7 FAFC FCFC FB00"                    /* .������������������������������. */
	$"00FA FAFA F6F9 FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA FAFA F9F7 FAFA FAFA FA00"                    /* .������������������������������. */
	$"0000 00FA FAFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFB FBFA FA00 0000 0000"                    /* ...������������������������..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
};

data 'ICN#' (1001, purgeable) {
	$"1FFF FC00 1000 0600 1070 0500 10C8 0480 10C8 0440 10D0 0420 10E0 07F0 11C7 8010"                    /* .��......p...�.�.�.@.�. .�.�.��. */
	$"1363 3810 1622 6410 1634 6410 1618 6810 173F 7010 13C6 E3D0 7001 B19C B003 111E"                    /* .c8.."d..4d...h..?p..���p.���... */
	$"9003 1A1A 9003 0C5A D003 9FDA B001 E39E 7000 001C 7FFF FFFC AAAA AAAE 8000 0002"                    /* �...�..Z�.���.��p....��������... */
	$"8000 0002 D555 5552 AAAA AAAE 7FFF FFFC 1FFF FFF0 1000 0010 1000 0010 1FFF FFF0"                    /* �...�UUR����.���.���.........��� */
	$"1FFF FC00 1FFF FE00 1FFF FF00 1FFF FF80 1FFF FFC0 1FFF FFE0 1FFF FFF0 1FFF FFF0"                    /* .��..��..��..���.���.���.���.��� */
	$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0 7FFF FFFC FFFF FFFE"                    /* .���.���.���.���.���.���.������� */
	$"FFFF FFFE FFFF FFFE FFFF FFFE FFFF FFFE 7FFF FFFC 7FFF FFFC FFFF FFFE FFFF FFFE"                    /* ����������������.���.����������� */
	$"FFFF FFFE FFFF FFFE FFFF FFFE 7FFF FFFC 1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"                    /* ������������.���.���.���.���.��� */
};

data 'ICN#' (1000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 FFFF FFFE 8000 0082 9000 00FF"                    /* ....................�����..��..� */
	$"8000 0081 8000 0085 8000 0081 8000 0085 8000 0081 8000 0081 8000 0085 8000 0081"                    /* �..��..��..��..��..��..��..��..� */
	$"FFFF FF81 9DC0 0EFD 9540 0AA1 95C0 0EBD 9500 02A1 9500 02BD 9500 02A1 F5FF FEBF"                    /* ������.��@���.��..��..��..����� */
	$"5580 1EA2 77FF FFBE 1FFF FFE0 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* U�.�w���.���.................... */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 FFFF FFFE FFFF FFFE FFFF FFFF"                    /* ....................������������ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"                    /* �������������������������������� */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"                    /* �������������������������������� */
	$"7FFF FFFE 7FFF FFFE 1FFF FFE0 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* .���.���.���.................... */
};

data 'ics#' (1001, purgeable) {
	$"3FE0 2830 3428 283C 3504 3244 2DA4 2044 E0AF A095 E06F FFFF 8001 FFFF 2004 3FFC"                    /* ?�(04((<5.2D-� D�����o���.�� .?� */
	$"3FE0 3FF0 3FF8 3FFC 3FFC 3FFC 3FFC 3FFC FFFF FFFF FFFF FFFF FFFF FFFF 3FFC 3FFC"                    /* ?�?�?�?�?�?�?�?�������������?�?� */
};

data 'ics#' (1000) {
	$"0000 0000 FFFE 800F 8009 8009 8009 8009 FFFF AC35 A815 AFF5 EFF7 3FFC 0000 0000"                    /* ....���.ĐĐĐĐ���5�.؞ԗ?�.... */
	$"0000 0000 FFFE FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF 3FFC 0000 0000"                    /* ....����������������������?�.... */
};

data 'ics4' (1001, purgeable) {
	$"00FF FFFF FFF0 0000 00FC 5C0C 0CFF 0000 00F5 C5C0 C0FC F000 00FC 500C 0CFF FF00"                    /* .�����...�\..�...������..�P..��. */
	$"00F5 C5C5 C0C0 CF00 00F5 0C5C 0D0C 0F00 00F0 55C5 D0D0 CF00 00FC 0C0C 0D0C 0F00"                    /* .������..�.\.....�U����..�...... */
	$"FFF0 C0C0 D0D0 DFFF FCFC 0C0C DC0D 0FCF FFF0 C0C0 CDD0 DFFF FFFF FFFF FFFF FFFF"                    /* ����������..�..�����խ���������� */
	$"FCCC CCCC CCCC CCCF FFFF FFFF FFFF FFFF 00F0 C0C0 C0C0 CF00 00FF FFFF FFFF FF00"                    /* ������æ��������.������..������. */
};

data 'ics4' (1000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000 DDDD DDDD DDDD DDD0 DCCC CCCC CCCC ECCD"                    /* ................��������������� */
	$"DCCC CCCC CCCC ECED DCCC CCCC CCCC ECCD DCCC CCCC CCCC ECDD DCCC CCCC CCCC ECDD"                    /* �������������������������������� */
	$"DDEE EEEE EEEE EEED DCDC E683 21ED CECD DCDC D683 210D CECD DCDC EEEE EEED CDCD"                    /* ������������!̑�����!.���������� */
	$"EEDC EDDD DEEE CEEE 00EE EEEE EEEE ED00 0000 0000 0000 0000 0000 0000 0000 0000"                    /* �����ӑ�.������................. */
};

data 'ics8' (1001, purgeable) {
	$"0000 FFFF FFFF FFFF FFFF FF00 0000 0000 0000 FFF5 B0F5 F5F5 F5F5 FFFF 0000 0000"                    /* ..���������.......����������.... */
	$"0000 FFB0 F5B0 F5F5 F5F5 FF2B FF00 0000 0000 FFF5 B0F5 F5F5 F5F5 FFFF FFFF 0000"                    /* ..���������+�.....������������.. */
	$"0000 FFB0 F5B0 F5B0 F5F5 F5F5 F5FF 0000 0000 FFB0 F5F5 B0F5 F5F9 F5F5 F5FF 0000"                    /* ..������������....������������.. */
	$"0000 FFF5 B0B0 F5B0 F9F5 F9F5 F5FF 0000 0000 FFF5 F5F5 F5F5 F5F9 F5F5 F5FF 0000"                    /* ..������������....������������.. */
	$"FFFF FFF5 F5F5 F5F5 F9F5 F9F5 F9FF FFFF FFF8 FFF5 F5F5 F5F5 F9F5 F5F9 F5FF F8FF"                    /* �������������������������������� */
	$"FFFF FFF5 F5F5 F5F5 F5F9 F9F5 F9FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"                    /* �������������������������������� */
	$"FFF8 F8F8 F8F8 F8F8 F8F8 F8F8 F8F8 F8FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"                    /* �������������������������������� */
	$"0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"                    /* ..������������....������������.. */
};

data 'ics8' (1000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
	$"FAFA FAFA FAFA FAFA FAFA FAFA FAFA FA00 FAF6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF8 F8FA"                    /* ���������������.���������������� */
	$"FAF6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 FBFA FAF6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F8FA"                    /* �������������������������������� */
	$"FAF6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F9FA FAF6 F6F6 F6F6 F6F6 F6F6 F6F6 FBF6 F9FA"                    /* �������������������������������� */
	$"FAFA FBFB FCFC FCFC FCFC FCFC FBFB FCFA FAF6 FAF6 FBEC E3D8 1705 FBFA F6FB F6FA"                    /* ���������������������ϓ�..������ */
	$"FAF6 F9F6 FAEC E3D8 1705 00FA F6FB F6FA FAF6 F9F6 FBFB FBFB FBFB FBFA F6FA F6FA"                    /* �����ϓ�...��������������������� */
	$"FBFC F9F6 FCFA FAFA FAFC FCFC F6FC FCFB 0000 FBFB FBFB FBFB FBFB FBFB FBFA 0000"                    /* ����������������..������������.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000"                    /* ................................ */
};
