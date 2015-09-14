﻿package
{
	import flash.display.*;
	import flash.external.*;
	import flash.events.*;
	
	public class SupportPage extends MovieClip
	{
		public function SupportPage():void
		{
			button_send.addEventListener(MouseEvent.CLICK, function(e:MouseEvent):void
			{
				if (text_email_address.text.length > 5 && text_message.text.length > 5 &&
					text_email_address.text.indexOf("@") >= 0 && text_email_address.text.indexOf(".") >= 0)
				{
					text_email_address.text = "";
					text_message.text = "";
					trace("message sent");
				}
				else
				{
					trace("please enter a valid email address and message");
				}
			});
		}
	}
}
