// Testing code from main loop


	/*
	ctr = 100;
	// Silly code to test multiple levels of grey on the LCD
	while(ctr--){
		// Write the msb
		for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
			lcd_buffer[rdx] = image_plane0[rdx];
		}
		lcd_render();
		delay_ms(10);

		// Write the lsb
		for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
			lcd_buffer[rdx] = image_plane1[rdx];
		}
		lcd_render();
		delay_ms(22);
	}
	 *
	 */

	//while(1);

//	// Silly code to test multiple levels of grey on the LCD
//	while(1){
//		// Write the msb
//		rdx = 0;
//		for(idx=0; idx<32; idx++){
//			lcd_buffer[rdx++] = image_plane0[idx];
//			if(rdx==16) rdx = 96;
//		}
//		lcd_render();
//		delay_ms(10);
//
//		// Write the lsb
//		rdx = 0;
//		for(idx=0; idx<32; idx++){
//			lcd_buffer[rdx++] = image_plane1[idx];
//			if(rdx==16) rdx = 96;
//		}
//		lcd_render();
//		delay_ms(16);
//	}
//
//	// Silly code to test multiple levels of grey on the LCD
//	while(1){
//		// Write the msb
//		for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
//			lcd_buffer[rdx] = (rdx & 0b1000)?0xff:0x00;
//		}
//		lcd_render();
//		delay_ms(16);
//		// Write the lsb
//		for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
//			lcd_buffer[rdx] = (rdx & 0b0100)?0xff:0x00;
//		}
//		lcd_render();
//		delay_ms(10);
//	}

/*	
 Some simple graphic routines
	for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
		lcd_buffer[rdx] = (uint8_t)rdx;
	}
	lcd_render();
	delay_ms(1000);
	
	lcd_clearLine(1);
	lcd_sendStr("What?");
	printf("Done");

	lcd_clearLine(4);
	lcd_clearLine(7);
	for(rdx=0; rdx<LCD_WIDTH; rdx++){
		lcd_set_pixel(rdx, 12, 1);
	}

	for(rdx=0; rdx<LCD_WIDTH; rdx+=8){
		lcd_draw_line(0,0,rdx,LCD_HEIGHT-1,1);
	}

	lcd_fill_rect(30,30,30,30,1);
	lcd_draw_circle(50,10,15,1);
	lcd_render();
	while(1){}
*/

/*
	for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
		g_lcdBuffer[rdx] = splash[rdx];
	}
	lcd_renderLines(0,5);
	rdx = 0;
*/

// Messing with volumes and drawing the sliders 
/*
	volume = 0; // Amount
	tdx = 1; // Direction
	while(1){
		volume += tdx;
		if(volume >= VOLUME_MAX){
			volume = VOLUME_MAX;
			tdx = -1;
		}
		if(volume <= VOLUME_MIN){
			volume = VOLUME_MIN;
			tdx = 1;
		}
		lcd_setCursor(1,0);
		xprintf("Vol:%2d   ", volume);
		//drawVerticalSlider(0, 1, 3, 15, VOLUME_RANGE, (uint8_t)(volume+VOLUME_MAX) );
		// drawVerticalSlider(5, 10, 3, 20, VOLUME_RANGE, (uint8_t)(volume+VOLUME_MAX) );
		drawVerticalSlider(4, 10, 3, 31, VOLUME_RANGE, (uint8_t)(volume+VOLUME_MAX) );
		lcd_render();
		delay_ms( 200 );
	};
*/
	/*
	while(1){
		// Alter the volumes
		for(idx=0; idx<=TRACKCOUNT; idx++){
			volume = g_song.volume[idx] + g_song.voldir[idx];
			
			if(volume >= VOLUME_MAX){
				volume = VOLUME_MAX;
				g_song.voldir[idx] = -1;
			}
			if(volume <= VOLUME_MIN){
				volume = VOLUME_MIN;
				g_song.voldir[idx] = 1;
			}
			g_song.volume[idx] = volume;
		}
		
		displayTrackVolumes(37,31);
		lcd_render();
		
		delay_ms(50);
		lcd_clearBuffer();
	}
*/