this.cookie = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "cookie";
	}

	// Create a cookie with the specified name and value.
	// The cookie expires at the end of the 20th century.
	this.set_cookie = function(name, value)
	{
		date = new Date();
		document.cookie = name + "=" + escape(value) + "; expires=Thu, 08 Jan 2088 23:59:59 GMT;";
	}

	// Retrieve the value of the cookie with the specified name.
	this.get_cookie = function(name)
	{
		// cookies are separated by semicolons
		var aCookie = document.cookie.split("; ");
		for (var i=0; i < aCookie.length; i++)
		{
			// a name/value pair (a crumb) is separated by an equal sign
			var aCrumb = aCookie[i].split("=");
			if (name == aCrumb[0]) 
			return unescape(aCrumb[1]);
		}
		
		// a cookie with the requested name does not exist
		return null;
	}

	this.del_cookie = function(name)
	{
		document.cookie = name + "=" + escape(value) + "; expires=Tue, 08 Jan 1980 23:59:59 GMT;";
	}

}



