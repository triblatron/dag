require "etc/commands"

root=
{
	foo=function(self)
		setPortValue(0, 3.0)
		return 0
	end,
	bar=function(self)
		setPortValue(0, -3.0)
		return 0
	end,
}
