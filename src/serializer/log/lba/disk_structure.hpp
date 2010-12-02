
#ifndef __SERIALIZER_LOG_LBA_DISK_STRUCTURE__
#define __SERIALIZER_LOG_LBA_DISK_STRUCTURE__

#include "arch/arch.hpp"
#include "serializer/log/extents/extent_manager.hpp"
#include "disk_format.hpp"
#include "disk_extent.hpp"

class lba_load_fsm_t;
class lba_writer_t;

class lba_disk_structure_t :
    public extent_t::read_callback_t
{
    friend class lba_load_fsm_t;
    friend class lba_writer_t;

public:
    // Create a new LBA
    lba_disk_structure_t(extent_manager_t *em, direct_file_t *file);
    
    // Load an existing LBA from disk
    struct load_callback_t {
        virtual void on_lba_load() = 0;
    };
    lba_disk_structure_t(extent_manager_t *em, direct_file_t *file, lba_shard_metablock_t *metablock);
    void set_load_callback(load_callback_t *lcb);
    
    // Put entries in an LBA and then call sync() to write to disk
    void add_entry(ser_block_id_t block_id, flagged_off64_t offset);
    struct sync_callback_t {
        virtual void on_lba_sync() = 0;
    };
    void sync(sync_callback_t *cb);
    
    // If you call read(), then the in_memory_index_t will be populated and then the read_callback_t
    // will be called when it is done.
    struct read_callback_t {
        virtual void on_lba_read() = 0;
    };
    void read(in_memory_index_t *index, read_callback_t *cb);
    
    void prepare_metablock(lba_shard_metablock_t *mb_out);
    
    void destroy();   // Delete both in memory and on disk
    void shutdown();   // Delete just in memory

    int num_entries_that_can_fit_in_an_extent() const;

private:
    extent_manager_t *em;
    direct_file_t *file;

public:
    extent_t *superblock_extent;   // Can be NULL
    off64_t superblock_offset;
    intrusive_list_t<lba_disk_extent_t> extents_in_superblock;
    lba_disk_extent_t *last_extent;

private:
    /* Used during the startup process */
    void on_extent_read();
    load_callback_t *start_callback;
    int startup_superblock_count;
    lba_superblock_t *startup_superblock_buffer;

    /* Use destroy() or shutdown() instead */
    ~lba_disk_structure_t() {}
};

#endif /* __SERIALIZER_LOG_LBA_DISK_STRUCTURE__ */
