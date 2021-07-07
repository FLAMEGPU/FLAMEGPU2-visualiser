#ifndef SRC_FLAMEGPU_VISUALISER_INTERFACE_RELOADABLE_H_
#define SRC_FLAMEGPU_VISUALISER_INTERFACE_RELOADABLE_H_

namespace flamegpu {
namespace visualiser {

/**
 * Interface for classes that can be reloaded
 * @see ShaderCore
 * @see EntityCore
 * @see TextureCore
 */
class Reloadable {
 public:
    /**
     * Public virtual destructor allows Pointers to this type to destroy subclasses correctly
     */
    virtual ~Reloadable() { }
    /**
     * Calling this reloads the subclass
     */
    virtual void reload() = 0;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_INTERFACE_RELOADABLE_H_
