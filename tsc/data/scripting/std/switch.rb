# -*- coding: utf-8 -*-
module Std

  # A switch that can be activated by jumping onto it. This class can either
  # be used to create entirely new switches or attach switch functionality to
  # an already switch sprite.
  #
  # Usage example (where the sprite with UID 14 is a pow sprite):
  #
  #   Std::Switch.new(UIDS[14]) do
  #     puts "Switch activated!"
  #   end
  #
  # Creating a new switch:
  #
  #   Std::Switch.new(x: 200, y: -200) do
  #     puts "Switch activated!"
  #   end
  class Switch

    # The underlying Sprite instance.
    attr_reader :sprite

    # The color of the pow switch.
    attr_reader :color

    # call-seq:
    #   new(){...}       → a_switch
    #   new(sprite){...} → a_switch
    #   new(opts){...}   → a_switch
    #
    # Create a new switch. If no parameters are given,
    # a switch is created at position (0|0).
    #
    # == Parameters
    # [sprite]
    #   Use this Sprite instance as the switch. The sprite should
    #   have its image set to "ground/underground/pow.png".
    # [opts]
    #   An option hash with the following keys:
    #   [sprite]
    #     Same as the +sprite+ parameter above.
    #   [x]
    #     Move the switch to this X position.
    #   [y]
    #     Move the switch to this Y position.
    #   [color]
    #     Color of the switch. Defaults to <tt>:blue</tt>.
    #   If not sprite is passed, setting +x+ and +y+ is recommended,
    #   this will be used as the creation position for the new sprite.
    #
    # If a block is given, use that block as the action to execute
    # when the switch gets activated.
    #
    # == Return value
    # The newly created instance.
    def initialize(opts = {}, &block)
      @activated = false
      @callback = block || lambda{puts "Dummy switch."}

      # If a single sprite was given, transform for uniform opts handling
      opts = {:sprite => opts} if opts.kind_of?(Sprite)
      opts[:color] ||= :blue

      @color = opts[:color]
      if opts[:sprite]
        @sprite = opts[:sprite]
        @sprite.start_at(opts[:x], opts[:y]) if opts[:x] && opts[:y]
      else
        opts[:x] ||= 0
        opts[:y] ||= 0
        @sprite = Sprite.new("ground/underground/switch/#{@color}.png")
        @sprite.massive_type = :massive
        @sprite.start_at(opts[:x], opts[:y])
        @sprite.show
      end

      @sprite.on_touch do |other|
        if !@activated && other.player? && other.velocity_y > 0
          Audio.play_sound("switch.ogg")
          @sprite.image = "ground/underground/switch/#{@color}_active.png"
          @activated = true
          @callback.call
          other.reset_on_ground
        end
      end
    end

    # Specify the action to execute when the switch is activated.
    def on_activate(&block)
      @callback = block
    end

    # Has the switch already been activated?
    def activated?
      @activated
    end

  end

end
